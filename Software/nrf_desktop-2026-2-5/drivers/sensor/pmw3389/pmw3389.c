/*
 * PMW3389 optical mouse sensor driver for nRF Desktop.
 *
 * Implements the Zephyr Sensor API so it integrates with the nRF Desktop
 * motion_sensor module:
 *   - sensor_trigger_set  (SENSOR_TRIG_DATA_READY on irq-gpios)
 *   - sensor_sample_fetch (motion burst read)
 *   - sensor_channel_get  (SENSOR_CHAN_POS_DX / SENSOR_CHAN_POS_DY)
 *   - sensor_attr_set     (PMW3389_ATTR_CPI, PMW3389_ATTR_REST_ENABLE)
 *
 * Key differences from the nRF Desktop PMW3360 driver:
 *   - No SROM firmware download required (PMW3389 has built-in firmware)
 *   - CPI range 50–16000 in steps of 50 (vs 100–12000 in steps of 100)
 *   - 16-bit CPI register: RESOLUTION_L (0x0E) + RESOLUTION_H (0x0F)
 *   - Product ID 0x47 (vs 0x42 for PMW3360)
 *
 * SPI timing constants are taken directly from the PMW3389 datasheet.
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT pixart_pmw3389

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <sensor/pmw3389.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pmw3389, CONFIG_PMW3389_LOG_LEVEL);


/* ---------------------------------------------------------------------------
 * SPI protocol
 * ---------------------------------------------------------------------------*/

/** Bit 7 of the address byte selects write mode. */
#define SPI_WRITE_BIT  BIT(7)


/* ---------------------------------------------------------------------------
 * Timing constants (all in microseconds unless noted)
 * Taken from PixArt PMW3389 datasheet Table 1 "Timing Specification"
 * ---------------------------------------------------------------------------*/

/** NCS-to-SCLK delay (min 120 ns; round up to 1 µs for busy-wait). */
#define T_NCS_SCLK       1

/** Address-to-data delay for single-register read (160 µs). */
#define T_SRAD           160

/** CS hold time after last SCLK edge for a read (120 ns → 1 µs). */
#define T_SCLK_NCS_READ  1

/** CS hold time after last SCLK edge for a write (35 µs). */
#define T_SCLK_NCS_WRITE 35

/** Inter-operation delay after a read (20 µs, same as T_SRW). */
#define T_SRX            20

/** Inter-operation delay after a write (180 µs, same as T_SWW). */
#define T_SWX            180

/** Address-to-data delay when entering motion burst (35 µs). */
#define T_SRAD_MOTBR     35

/** Burst-exit / NCS-deassert hold time (500 ns → 1 µs). */
#define T_BEXIT          1


/* ---------------------------------------------------------------------------
 * Register map (PMW3389 datasheet Section 7)
 * ---------------------------------------------------------------------------*/

#define PMW3389_REG_PRODUCT_ID      0x00
#define PMW3389_REG_REVISION_ID     0x01
#define PMW3389_REG_MOTION          0x02
#define PMW3389_REG_DELTA_X_L       0x03
#define PMW3389_REG_DELTA_X_H       0x04
#define PMW3389_REG_DELTA_Y_L       0x05
#define PMW3389_REG_DELTA_Y_H       0x06
#define PMW3389_REG_CONTROL         0x0D
#define PMW3389_REG_RESOLUTION_L    0x0E  /* CPI [7:0] */
#define PMW3389_REG_RESOLUTION_H    0x0F  /* CPI [8] (bit 0 only) */
#define PMW3389_REG_CONFIG2         0x10
#define PMW3389_REG_ANGLE_TUNE      0x11
#define PMW3389_REG_POWER_UP_RESET  0x3A
#define PMW3389_REG_SHUTDOWN        0x3B
#define PMW3389_REG_MOTION_BURST    0x50
#define PMW3389_REG_LIFT_CONFIG     0x63

/* MOTION register bits */
#define PMW3389_MOTION_MOT_BIT      BIT(7)  /* Set when motion is detected */

/* CONFIG2 register bits */
#define PMW3389_REST_EN_BIT         BIT(5)  /* Enable power-save (REST) modes */
#define PMW3389_REST_EN_POS         5

/* Software power-up reset command */
#define PMW3389_POWER_UP_RESET_CMD  0x5A


/* ---------------------------------------------------------------------------
 * Sensor identification
 * ---------------------------------------------------------------------------*/

#define PMW3389_PRODUCT_ID  0x47  /* Expected value in PRODUCT_ID register */


/* ---------------------------------------------------------------------------
 * CPI (resolution) configuration
 * Register value = (CPI / 50) - 1   → 9-bit result stored in H:L
 * ---------------------------------------------------------------------------*/

#define PMW3389_MAX_CPI   16000
#define PMW3389_MIN_CPI   50
#define PMW3389_CPI_STEP  50


/* ---------------------------------------------------------------------------
 * Motion burst layout
 * Reading starting from MOTION_BURST register yields these 6 bytes:
 * [0] Motion    [1] Observation
 * [2] Delta_X_L [3] Delta_X_H
 * [4] Delta_Y_L [5] Delta_Y_H
 * (Additional bytes follow but are not needed here.)
 * ---------------------------------------------------------------------------*/

#define PMW3389_BURST_SIZE  6
#define PMW3389_DX_POS      2  /* index of Delta_X_L in burst buffer */
#define PMW3389_DY_POS      4  /* index of Delta_Y_L in burst buffer */


/* ---------------------------------------------------------------------------
 * Helper macros for converting sensor_value to driver values
 * ---------------------------------------------------------------------------*/

#define PMW3389_SVALUE_TO_CPI(sv)   ((uint32_t)(sv).val1)
#define PMW3389_SVALUE_TO_BOOL(sv)  ((sv).val1 != 0)


/* ---------------------------------------------------------------------------
 * Asynchronous initialisation state machine
 * The real init work runs in a delayable work item so we never block the
 * system work queue for more than a few microseconds per step.
 *
 * Four-step sequence (per PMW3389 datasheet power-up procedure):
 *   RESET_ASSERT   – drive NRESET low (hardware reset), ensure CS high
 *   RESET_DEASSERT – release NRESET, pulse CS low→high (SPI port reset)
 *   SW_RESET       – write 0x5A to POWER_UP_RESET register
 *   CONFIGURE      – read product ID, set CPI, enable REST modes
 *
 * Delays are scheduled *between* steps, so the work queue is only blocked
 * for a few microseconds of SPI activity per callback.
 * ---------------------------------------------------------------------------*/

enum async_init_step {
	/** Assert NRESET (hardware reset) and ensure CS is deasserted. */
	ASYNC_INIT_STEP_RESET_ASSERT,
	/** Release NRESET and pulse CS to reset the sensor SPI port. */
	ASYNC_INIT_STEP_RESET_DEASSERT,
	/** Write 0x5A to POWER_UP_RESET register (software reset). */
	ASYNC_INIT_STEP_SW_RESET,
	/** Read product ID, set CPI, enable REST modes. */
	ASYNC_INIT_STEP_CONFIGURE,

	ASYNC_INIT_STEP_COUNT,
};

/** Maximum number of init retries before giving up (allows sensor to power on late). */
#define PMW3389_INIT_MAX_RETRIES  5
/** Delay in ms before retrying a failed init. */
#define PMW3389_INIT_RETRY_DELAY_MS  500

/**
 * Millisecond delays *between* steps (i.e. the delay scheduled after the
 * previous step completes before the next step runs).
 */
static const int32_t async_init_delay[ASYNC_INIT_STEP_COUNT] = {
	[ASYNC_INIT_STEP_RESET_ASSERT]   = 1,   /* start quickly after boot */
	[ASYNC_INIT_STEP_RESET_DEASSERT] = 10,  /* hold NRESET low for >= 10 ms */
	[ASYNC_INIT_STEP_SW_RESET]       = 50,  /* 50 ms boot time after NRESET release */
	[ASYNC_INIT_STEP_CONFIGURE]      = 50,  /* 50 ms after POWER_UP_RESET write */
};


/* ---------------------------------------------------------------------------
 * Driver data and config structs
 * ---------------------------------------------------------------------------*/

struct pmw3389_data {
	const struct device          *dev;
	struct gpio_callback          irq_gpio_cb;
	struct k_spinlock             lock;

	int16_t x;
	int16_t y;

	sensor_trigger_handler_t      data_ready_handler;
	struct k_work                 trigger_handler_work;
	struct k_work_delayable       init_work;

	enum async_init_step          async_init_step;
	int                           err;
	uint8_t                       retry_count;
	bool                          ready;
	bool                          init_failed;
	bool                          last_read_burst;
};

struct pmw3389_config {
	/** SPI bus spec (frequency, mode, slave address). */
	struct spi_dt_spec  spi;
	/** CS GPIO extracted from the parent SPI bus cs-gpios property.
	 *  Used for manual CS control during the power-up sequence. */
	struct gpio_dt_spec cs_gpio;
	/** MOTION (data-ready) interrupt pin. */
	struct gpio_dt_spec irq_gpio;
        /** NRESET pin – optional.  When present, driven HIGH during init to
         *  release the sensor from hardware reset before the SPI sequence. */
        struct gpio_dt_spec reset_gpio;
};

static int pmw3389_async_init_reset_assert(const struct device *dev);
static int pmw3389_async_init_reset_deassert(const struct device *dev);
static int pmw3389_async_init_sw_reset(const struct device *dev);
static int pmw3389_async_init_configure(const struct device *dev);

static int (* const async_init_fn[ASYNC_INIT_STEP_COUNT])(const struct device *dev) = {
	[ASYNC_INIT_STEP_RESET_ASSERT]   = pmw3389_async_init_reset_assert,
	[ASYNC_INIT_STEP_RESET_DEASSERT] = pmw3389_async_init_reset_deassert,
	[ASYNC_INIT_STEP_SW_RESET]       = pmw3389_async_init_sw_reset,
	[ASYNC_INIT_STEP_CONFIGURE]      = pmw3389_async_init_configure,
};


/* ---------------------------------------------------------------------------
 * SPI helpers
 *
 * CS is managed by the Zephyr SPI framework via SPI_HOLD_ON_CS and
 * SPI_LOCK_ON flags.  spi_write_dt / spi_read_dt keep CS asserted,
 * and spi_release_dt deasserts CS and releases the bus lock.
 *
 * This approach matches the ZMK PMW3389 reference driver and avoids
 * conflicts between manual GPIO CS and framework CS management.
 * ---------------------------------------------------------------------------*/

/**
 * Read a single register value.
 *
 * Protocol:
 *   CS↓ → write addr → wait T_SRAD → read byte → wait T_SCLK_NCS_READ → CS↑
 *   → wait T_SRX
 */
static int reg_read(const struct device *dev, uint8_t reg, uint8_t *val)
{
	struct pmw3389_data *data = dev->data;
	const struct pmw3389_config *config = dev->config;

	__ASSERT_NO_MSG((reg & SPI_WRITE_BIT) == 0);

	/* Send address byte.  SPI_HOLD_ON_CS keeps CS asserted. */
	const struct spi_buf tx_buf = { .buf = &reg, .len = 1 };
	const struct spi_buf_set tx  = { .buffers = &tx_buf, .count = 1 };

	int err = spi_write_dt(&config->spi, &tx);
	if (err) {
		LOG_ERR("reg_read: SPI write addr failed (err %d)", err);
		spi_release_dt(&config->spi);
		return err;
	}

	/* Address-to-data delay (160 µs per datasheet). */
	k_busy_wait(T_SRAD);

	/* Read data byte.  CS is still asserted (SPI_HOLD_ON_CS). */
	struct spi_buf rx_buf = { .buf = val, .len = 1 };
	const struct spi_buf_set rx  = { .buffers = &rx_buf, .count = 1 };

	err = spi_read_dt(&config->spi, &rx);
	if (err) {
		LOG_ERR("reg_read: SPI read failed (err %d)", err);
	}

	/* Hold CS for T_SCLK_NCS_READ then release (deassert CS). */
	k_busy_wait(T_SCLK_NCS_READ);
	spi_release_dt(&config->spi);
	k_busy_wait(T_SRX);

	data->last_read_burst = false;

	return err;
}

/**
 * Write a single register value.
 *
 * Protocol:
 *   CS↓ → write (addr|0x80, val) → wait T_SCLK_NCS_WRITE → CS↑ → wait T_SWX
 */
static int reg_write(const struct device *dev, uint8_t reg, uint8_t val)
{
	struct pmw3389_data *data = dev->data;
	const struct pmw3389_config *config = dev->config;

	__ASSERT_NO_MSG((reg & SPI_WRITE_BIT) == 0);

	uint8_t buf[2] = { SPI_WRITE_BIT | reg, val };
	const struct spi_buf tx_buf = { .buf = buf, .len = sizeof(buf) };
	const struct spi_buf_set tx  = { .buffers = &tx_buf, .count = 1 };

	int err = spi_write_dt(&config->spi, &tx);
	if (err) {
		LOG_ERR("reg_write: SPI write failed (err %d)", err);
	}

	/* Hold CS for T_SCLK_NCS_WRITE then release. */
	k_busy_wait(T_SCLK_NCS_WRITE);
	spi_release_dt(&config->spi);
	k_busy_wait(T_SWX);

	data->last_read_burst = false;

	return err;
}

/**
 * Read @p burst_size bytes starting at the MOTION_BURST register.
 *
 * Protocol:
 *   If not already in burst mode, write any value to MOTION_BURST first.
 *   CS↓ → write MOTION_BURST addr → wait T_SRAD_MOTBR → read burst → CS↑
 *   → wait T_BEXIT
 */
static int motion_burst_read(const struct device *dev, uint8_t *buf,
			     size_t burst_size)
{
	struct pmw3389_data *data = dev->data;
	const struct pmw3389_config *config = dev->config;

	/* Enter burst mode: write any value to MOTION_BURST (only if needed). */
	if (!data->last_read_burst) {
		int err = reg_write(dev, PMW3389_REG_MOTION_BURST, 0x00);
		if (err) {
			return err;
		}
	}

	/* Send burst register address.  SPI_HOLD_ON_CS keeps CS asserted. */
	uint8_t reg_addr = PMW3389_REG_MOTION_BURST;
	const struct spi_buf tx_buf = { .buf = &reg_addr, .len = 1 };
	const struct spi_buf_set tx  = { .buffers = &tx_buf, .count = 1 };

	int err = spi_write_dt(&config->spi, &tx);
	if (err) {
		LOG_ERR("motion_burst_read: SPI write addr failed (err %d)", err);
		spi_release_dt(&config->spi);
		return err;
	}

	k_busy_wait(T_SRAD_MOTBR);

	/* Read burst data. */
	struct spi_buf rx_buf = { .buf = buf, .len = burst_size };
	const struct spi_buf_set rx  = { .buffers = &rx_buf, .count = 1 };

	err = spi_read_dt(&config->spi, &rx);
	if (err) {
		LOG_ERR("motion_burst_read: SPI read failed (err %d)", err);
	}

	spi_release_dt(&config->spi);
	k_busy_wait(T_BEXIT);

	data->last_read_burst = (err == 0);

	return err;
}


/* ---------------------------------------------------------------------------
 * CPI and REST helper functions
 * ---------------------------------------------------------------------------*/

/**
 * Set the sensor CPI (counts per inch).
 * Register value = (cpi / 50) - 1, stored as 9-bit value in H:L.
 */
static int update_cpi(const struct device *dev, uint32_t cpi)
{
	if ((cpi > PMW3389_MAX_CPI) || (cpi < PMW3389_MIN_CPI)) {
		LOG_ERR("CPI %u out of range [%u, %u]",
			cpi, PMW3389_MIN_CPI, PMW3389_MAX_CPI);
		return -EINVAL;
	}

	uint16_t reg_val = (uint16_t)(cpi / PMW3389_CPI_STEP) - 1;

	LOG_INF("Setting CPI to %u (register value 0x%04x)", cpi, reg_val);

	int err = reg_write(dev, PMW3389_REG_RESOLUTION_H,
			    (uint8_t)((reg_val >> 8) & 0x01));
	if (!err) {
		err = reg_write(dev, PMW3389_REG_RESOLUTION_L,
				(uint8_t)(reg_val & 0xFF));
	}

	if (err) {
		LOG_ERR("Failed to set CPI (err %d)", err);
	}

	return err;
}

/**
 * Enable or disable the sensor's power-save REST modes via CONFIG2 register.
 */
static int toggle_rest_modes(const struct device *dev, bool enable)
{
	uint8_t value = 0;
	int err = reg_read(dev, PMW3389_REG_CONFIG2, &value);

	if (err) {
		LOG_ERR("Failed to read CONFIG2 (err %d)", err);
		return err;
	}

	WRITE_BIT(value, PMW3389_REST_EN_POS, enable);

	LOG_DBG("%sabling REST modes (CONFIG2 = 0x%02x)",
		enable ? "En" : "Dis", value);

	err = reg_write(dev, PMW3389_REG_CONFIG2, value);
	if (err) {
		LOG_ERR("Failed to write CONFIG2 (err %d)", err);
	}

	return err;
}


/* ---------------------------------------------------------------------------
 * Asynchronous initialisation
 * ---------------------------------------------------------------------------*/

/**
 * Step 1 (RESET_ASSERT): Put the sensor into hardware reset.
 *
 * Configure CS GPIO as output HIGH (deasserted).  Drive NRESET active
 * (LOW) to hold the sensor in reset.  The work-queue scheduler will
 * wait 10 ms (RESET_DEASSERT delay) before releasing NRESET.
 */
static int pmw3389_async_init_reset_assert(const struct device *dev)
{
	const struct pmw3389_config *config = dev->config;
	int err;

	/* Configure CS GPIO so we can pulse it manually during init.
	 * Start deasserted (HIGH). */
	err = gpio_pin_configure_dt(&config->cs_gpio, GPIO_OUTPUT_INACTIVE);
	if (err) {
		LOG_ERR("Failed to configure CS GPIO (err %d)", err);
		return err;
	}

	/* Drive NRESET active (LOW) – sensor enters hardware reset. */
	if (config->reset_gpio.port != NULL) {
		err = gpio_pin_set_dt(&config->reset_gpio, 1); /* 1 = active */
		if (err) {
			LOG_ERR("Failed to assert NRESET (err %d)", err);
			return err;
		}
		LOG_DBG("NRESET asserted – sensor in hardware reset");
	}

	return 0;
}

/**
 * Step 2 (RESET_DEASSERT): Release hardware reset, reset SPI port.
 *
 * Called 10 ms after Step 1.  Releases NRESET (HIGH), then pulses CS
 * LOW→HIGH to reset the sensor's SPI port per datasheet Section 4.
 * After returning, the work-queue waits 50 ms for the sensor to boot.
 */
static int pmw3389_async_init_reset_deassert(const struct device *dev)
{
	const struct pmw3389_config *config = dev->config;

	/* Release NRESET (drive inactive = HIGH). */
	if (config->reset_gpio.port != NULL) {
		gpio_pin_set_dt(&config->reset_gpio, 0); /* 0 = inactive */
		LOG_DBG("NRESET released – sensor booting");
	}

	/* Per datasheet: pulse NCS LOW then HIGH to reset the SPI port. */
	gpio_pin_set_dt(&config->cs_gpio, 1); /* active = LOW */
	k_busy_wait(50);                       /* brief CS pulse */
	gpio_pin_set_dt(&config->cs_gpio, 0); /* inactive = HIGH */

	LOG_DBG("SPI port reset (CS pulsed LOW -> HIGH)");

	return 0;
}

/**
 * Step 3 (SW_RESET): Software power-up reset.
 *
 * Called 50 ms after NRESET release.  Writes 0x5A to POWER_UP_RESET.
 * After returning, the work-queue waits 50 ms for the reset to complete.
 */
static int pmw3389_async_init_sw_reset(const struct device *dev)
{
	int err = reg_write(dev, PMW3389_REG_POWER_UP_RESET,
			    PMW3389_POWER_UP_RESET_CMD);
	if (err) {
		LOG_ERR("POWER_UP_RESET write failed (err %d)", err);
	}

	return err;
}

/**
 * Step 4 (CONFIGURE): Read product ID, set CPI, enable REST modes.
 *
 * 1. Read and discard registers 0x02–0x06 (clears motion state).
 * 2. Verify product ID.
 * 3. Set CPI to CONFIG_DESKTOP_MOTION_SENSOR_CPI (or default 1600).
 * 4. Enable REST power-save modes.
 */
static int pmw3389_async_init_configure(const struct device *dev)
{
	int err = 0;

	/* Read and discard motion-related registers to clear internal state. */
	for (uint8_t reg = PMW3389_REG_MOTION;
	     (reg <= PMW3389_REG_DELTA_Y_H) && !err; reg++) {
		uint8_t dummy;
		err = reg_read(dev, reg, &dummy);
	}
	if (err) {
		LOG_ERR("Failed to read motion registers during init (err %d)", err);
		return err;
	}

	/* Verify product ID. */
	uint8_t product_id = 0;
	uint8_t revision_id = 0;

	(void)reg_read(dev, PMW3389_REG_PRODUCT_ID, &product_id);
	(void)reg_read(dev, PMW3389_REG_REVISION_ID, &revision_id);

	LOG_INF("PMW3389 product_id=0x%02x revision_id=0x%02x",
		product_id, revision_id);

	if (product_id != PMW3389_PRODUCT_ID) {
		LOG_ERR("Product ID mismatch: got 0x%02x, expected 0x%02x. "
			"Check SPI wiring (SCK/MOSI/MISO/CS pins and pinctrl).",
			product_id, PMW3389_PRODUCT_ID);
		return -EIO;
	}

	/* Set default CPI (1600 DPI is a reasonable default). */
	uint32_t default_cpi = (CONFIG_DESKTOP_MOTION_SENSOR_CPI != 0)
			       ? CONFIG_DESKTOP_MOTION_SENSOR_CPI : 1600;

	err = update_cpi(dev, default_cpi);
	if (err) {
		LOG_ERR("Failed to set default CPI (err %d)", err);
		return err;
	}

	/* Enable REST (power-save) modes. */
	err = toggle_rest_modes(dev, true);
	if (err) {
		LOG_ERR("Failed to enable REST modes (err %d)", err);
		return err;
	}

	LOG_INF("PMW3389 initialised at %u CPI", default_cpi);

	return 0;
}

/**
 * Work callback: advances the async init state machine one step at a time.
 *
 * On failure the state machine resets to POWER_UP and retries up to
 * PMW3389_INIT_MAX_RETRIES times (PMW3389_INIT_RETRY_DELAY_MS apart).
 * This allows the device to recover if the sensor is powered on after boot.
 */
static void pmw3389_async_init_work_cb(struct k_work *work)
{
	struct k_work_delayable *work_delayable =
		(struct k_work_delayable *)work;
	struct pmw3389_data *data =
		CONTAINER_OF(work_delayable, struct pmw3389_data, init_work);
	const struct device *dev = data->dev;

	LOG_DBG("PMW3389 async init step %d (retry %d/%d)",
		data->async_init_step, data->retry_count,
		PMW3389_INIT_MAX_RETRIES);

	data->err = async_init_fn[data->async_init_step](dev);
	if (data->err) {
		LOG_ERR("PMW3389 init step %d failed (err %d)",
			data->async_init_step, data->err);

		/* Retry: reset the state machine and reschedule from step 0. */
		if (data->retry_count < PMW3389_INIT_MAX_RETRIES) {
			data->retry_count++;
			data->async_init_step = ASYNC_INIT_STEP_RESET_ASSERT;
			LOG_WRN("PMW3389 init retry %d/%d in %d ms",
				data->retry_count, PMW3389_INIT_MAX_RETRIES,
				PMW3389_INIT_RETRY_DELAY_MS);
			k_work_schedule(&data->init_work,
					K_MSEC(PMW3389_INIT_RETRY_DELAY_MS));
		} else {
			data->init_failed = true;
			LOG_ERR("PMW3389 init failed after %d retries. "
				"Check SPI wiring (SCK/MOSI/MISO/CS/NRESET) "
				"and that the sensor is powered.",
				PMW3389_INIT_MAX_RETRIES);
		}
		return;
	}

	data->async_init_step++;

	if (data->async_init_step == ASYNC_INIT_STEP_COUNT) {
		data->ready = true;
		LOG_INF("PMW3389 ready (after %d attempt(s))",
			data->retry_count + 1);
	} else {
		k_work_schedule(&data->init_work,
				K_MSEC(async_init_delay[data->async_init_step]));
	}
}


/* ---------------------------------------------------------------------------
 * IRQ / trigger handling
 * ---------------------------------------------------------------------------*/

/**
 * Bottom-half work item: deliver the data-ready trigger to the application.
 *
 * Mirrors the PMW3360 pattern: read handler under spinlock, call it, then
 * re-enable the GPIO interrupt if the handler is still registered.
 */
static void trigger_handler_work_cb(struct k_work *work)
{
	sensor_trigger_handler_t handler;
	int err = 0;
	struct pmw3389_data *data =
		CONTAINER_OF(work, struct pmw3389_data, trigger_handler_work);
	const struct device *dev = data->dev;
	const struct pmw3389_config *config = dev->config;

	k_spinlock_key_t key = k_spin_lock(&data->lock);
	handler = data->data_ready_handler;
	k_spin_unlock(&data->lock, key);

	if (!handler) {
		return;
	}

	struct sensor_trigger trig = {
		.type = SENSOR_TRIG_DATA_READY,
		.chan = SENSOR_CHAN_ALL,
	};

	handler(dev, &trig);

	key = k_spin_lock(&data->lock);
	if (data->data_ready_handler) {
		err = gpio_pin_interrupt_configure_dt(&config->irq_gpio,
						      GPIO_INT_LEVEL_ACTIVE);
	}
	k_spin_unlock(&data->lock, key);

	if (unlikely(err)) {
		LOG_ERR("Cannot re-enable IRQ (err %d)", err);
	}
}

/** GPIO interrupt service routine – fires when MOTION pin goes active. */
static void irq_handler(const struct device *gpiob,
			struct gpio_callback *cb,
			uint32_t pins)
{
	struct pmw3389_data *data =
		CONTAINER_OF(cb, struct pmw3389_data, irq_gpio_cb);
	const struct device *dev = data->dev;
	const struct pmw3389_config *config = dev->config;

	/* Disable the interrupt until the application re-arms it via trigger_set. */
	gpio_pin_interrupt_configure_dt(&config->irq_gpio, GPIO_INT_DISABLE);

	k_work_submit(&data->trigger_handler_work);
}

/** Configure the MOTION pin interrupt and register/remove the trigger callback. */
static int pmw3389_init_irq(const struct device *dev)
{
	struct pmw3389_data *data = dev->data;
	const struct pmw3389_config *config = dev->config;

	if (!device_is_ready(config->irq_gpio.port)) {
		LOG_ERR("IRQ GPIO device not ready");
		return -ENODEV;
	}

	int err = gpio_pin_configure_dt(&config->irq_gpio, GPIO_INPUT);
	if (err) {
		LOG_ERR("Cannot configure IRQ GPIO (err %d)", err);
		return err;
	}

	gpio_init_callback(&data->irq_gpio_cb, irq_handler,
			   BIT(config->irq_gpio.pin));

	err = gpio_add_callback(config->irq_gpio.port, &data->irq_gpio_cb);
	if (err) {
		LOG_ERR("Cannot add IRQ GPIO callback (err %d)", err);
	}

	return err;
}


/* ---------------------------------------------------------------------------
 * Sensor API implementation
 * ---------------------------------------------------------------------------*/

/**
 * sensor_sample_fetch – read motion data from the sensor via burst.
 *
 * Stores dx/dy in data->x and data->y so they can be retrieved by
 * sensor_channel_get without an extra SPI transaction.
 */
static int pmw3389_sample_fetch(const struct device *dev,
				enum sensor_channel chan)
{
	struct pmw3389_data *data = dev->data;
	uint8_t buf[PMW3389_BURST_SIZE];

	if (unlikely(chan != SENSOR_CHAN_ALL)) {
		return -ENOTSUP;
	}

	if (unlikely(!data->ready)) {
		if (data->init_failed) {
			return -ENODEV;
		}
		LOG_DBG("Device not yet initialised");
		return -EBUSY;
	}

	int err = motion_burst_read(dev, buf, sizeof(buf));
	if (err) {
		return err;
	}

	/*
	 * Delta X/Y are stored little-endian at DX_POS/DY_POS.
	 * sys_get_le16 handles the two-byte sign-extended decode.
	 */
	data->x = (int16_t)sys_get_le16(&buf[PMW3389_DX_POS]);
	data->y = (int16_t)sys_get_le16(&buf[PMW3389_DY_POS]);

	return 0;
}

/**
 * sensor_channel_get – return the last fetched X or Y displacement.
 */
static int pmw3389_channel_get(const struct device *dev,
			       enum sensor_channel chan,
			       struct sensor_value *val)
{
	struct pmw3389_data *data = dev->data;

	if (unlikely(!data->ready)) {
		if (data->init_failed) {
			return -ENODEV;
		}
		LOG_DBG("Device not yet initialised");
		return -EBUSY;
	}

	switch (chan) {
	case SENSOR_CHAN_POS_DX:
		val->val1 = data->x;
		val->val2 = 0;
		break;

	case SENSOR_CHAN_POS_DY:
		val->val1 = data->y;
		val->val2 = 0;
		break;

	default:
		return -ENOTSUP;
	}

	return 0;
}

/**
 * sensor_trigger_set – configure the MOTION GPIO interrupt.
 *
 * Only SENSOR_TRIG_DATA_READY on SENSOR_CHAN_ALL is supported.
 * Passing handler=NULL disables the interrupt.
 */
static int pmw3389_trigger_set(const struct device *dev,
			       const struct sensor_trigger *trig,
			       sensor_trigger_handler_t handler)
{
	struct pmw3389_data *data = dev->data;
	const struct pmw3389_config *config = dev->config;

	if (unlikely(trig->type != SENSOR_TRIG_DATA_READY)) {
		return -ENOTSUP;
	}

	if (unlikely(trig->chan != SENSOR_CHAN_ALL)) {
		return -ENOTSUP;
	}

	if (unlikely(!data->ready)) {
		if (data->init_failed) {
			return -ENODEV;
		}
		LOG_DBG("Device not yet initialised");
		return -EBUSY;
	}

	k_spinlock_key_t key = k_spin_lock(&data->lock);

	int err;

	if (handler) {
		err = gpio_pin_interrupt_configure_dt(&config->irq_gpio,
						      GPIO_INT_LEVEL_ACTIVE);
	} else {
		err = gpio_pin_interrupt_configure_dt(&config->irq_gpio,
						      GPIO_INT_DISABLE);
	}

	if (!err) {
		data->data_ready_handler = handler;
	}

	k_spin_unlock(&data->lock, key);

	return err;
}

/**
 * sensor_attr_set – set a runtime-configurable sensor attribute.
 *
 * Supported attributes:
 *   PMW3389_ATTR_CPI        – change tracking resolution (50–16000 CPI)
 *   PMW3389_ATTR_REST_ENABLE – enable/disable power-save REST modes
 */
static int pmw3389_attr_set(const struct device *dev,
			    enum sensor_channel chan,
			    enum sensor_attribute attr,
			    const struct sensor_value *val)
{
	struct pmw3389_data *data = dev->data;

	if (unlikely(chan != SENSOR_CHAN_ALL)) {
		return -ENOTSUP;
	}

	if (unlikely(!data->ready)) {
		if (data->init_failed) {
			return -ENODEV;
		}
		LOG_DBG("Device not yet initialised");
		return -EBUSY;
	}

	switch ((uint32_t)attr) {
	case PMW3389_ATTR_CPI:
		return update_cpi(dev, PMW3389_SVALUE_TO_CPI(*val));

	case PMW3389_ATTR_REST_ENABLE:
		return toggle_rest_modes(dev, PMW3389_SVALUE_TO_BOOL(*val));

	default:
		LOG_ERR("Unknown attribute %d", attr);
		return -ENOTSUP;
	}
}

static const struct sensor_driver_api pmw3389_driver_api = {
	.sample_fetch = pmw3389_sample_fetch,
	.channel_get  = pmw3389_channel_get,
	.trigger_set  = pmw3389_trigger_set,
	.attr_set     = pmw3389_attr_set,
};


/* ---------------------------------------------------------------------------
 * Driver initialisation
 * ---------------------------------------------------------------------------*/

static int pmw3389_init(const struct device *dev)
{
	struct pmw3389_data *data = dev->data;
	const struct pmw3389_config *config = dev->config;

	data->dev = dev;
	data->async_init_step = ASYNC_INIT_STEP_RESET_ASSERT;

	if (!spi_is_ready_dt(&config->spi)) {
		LOG_ERR("SPI bus not ready");
		return -ENODEV;
	}

	if (!device_is_ready(config->cs_gpio.port)) {
		LOG_ERR("CS GPIO device not ready");
		return -ENODEV;
	}

	/* Configure NRESET as output active (LOW) to hold the sensor in
	 * hardware reset from the start.  The async init will release it
	 * after a proper delay.  Pin is optional in DTS.
	 */
	if (config->reset_gpio.port != NULL) {
		if (!device_is_ready(config->reset_gpio.port)) {
			LOG_ERR("NRESET GPIO device not ready");
			return -ENODEV;
		}
		int reset_err = gpio_pin_configure_dt(&config->reset_gpio,
						      GPIO_OUTPUT_ACTIVE);
		if (reset_err) {
			LOG_ERR("Cannot configure NRESET GPIO (err %d)",
				reset_err);
			return reset_err;
		}
		LOG_DBG("NRESET driven LOW – sensor held in reset");
	}

	int err = pmw3389_init_irq(dev);
	if (err) {
		return err;
	}

	k_work_init(&data->trigger_handler_work, trigger_handler_work_cb);
	k_work_init_delayable(&data->init_work, pmw3389_async_init_work_cb);

	k_work_schedule(&data->init_work,
			K_MSEC(async_init_delay[ASYNC_INIT_STEP_RESET_ASSERT]));

	return 0;
}


/* ---------------------------------------------------------------------------
 * Device instantiation macro
 *
 * SPI configuration:
 *   Mode 3 (CPOL=1 CPHA=1), MSB first, 8-bit words – as required by PMW3389.
 *   SPI_HOLD_ON_CS keeps CS asserted across multi-part read transactions.
 *   SPI_LOCK_ON prevents other threads from interleaving SPI traffic.
 *   CS GPIO comes from the bus's cs-gpios DT property via SPI_DT_SPEC_INST_GET.
 * ---------------------------------------------------------------------------*/

#define PMW3389_SPI_OP \
	(SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_MODE_CPOL | \
	 SPI_MODE_CPHA | SPI_TRANSFER_MSB | SPI_HOLD_ON_CS | SPI_LOCK_ON)

#define PMW3389_DEFINE(n)						\
	static struct pmw3389_data pmw3389_data_##n;			\
									\
	static const struct pmw3389_config pmw3389_config_##n = {	\
		.spi = SPI_DT_SPEC_INST_GET(n, PMW3389_SPI_OP, 0),	\
		.cs_gpio    = SPI_CS_GPIOS_DT_SPEC_GET(DT_DRV_INST(n)),	\
		.irq_gpio   = GPIO_DT_SPEC_INST_GET(n, irq_gpios),		\
		.reset_gpio = GPIO_DT_SPEC_INST_GET_OR(n, reset_gpios, {0}),	\
	};								\
									\
	DEVICE_DT_INST_DEFINE(n,					\
		pmw3389_init, NULL,					\
		&pmw3389_data_##n, &pmw3389_config_##n,			\
		POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,		\
		&pmw3389_driver_api);

DT_INST_FOREACH_STATUS_OKAY(PMW3389_DEFINE)
