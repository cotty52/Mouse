# PMW3389 Driver Integration – Summary

**Date:** February 20, 2026

---

## Investigation

### PMW3360 (nRF Desktop built-in driver)
Location: `/home/christian/ncs/v3.2.1/nrf/drivers/sensor/pmw3360/`

- Implements the **Zephyr Sensor API**: `sample_fetch`, `channel_get`, `trigger_set`, `attr_set`
- Requires a **SROM firmware download** (~4 KB blob in `pmw3360_priv.c`) on every power-up
- CPI range: 100–12000 in steps of 100 (8-bit register `CONFIG1`)
- Product ID: `0x42`
- Multi-step async init state machine: POWER_UP → FW_LOAD_START → FW_LOAD_CONTINUE → FW_LOAD_VERIFY → CONFIGURE
- nRF Desktop `motion_sensor.c` binds to it via `DEVICE_DT_GET_ONE(pixart_pmw3360)`
- Public header: `/home/christian/ncs/v3.2.1/nrf/include/sensor/pmw3360.h`
- DTS binding: `/home/christian/ncs/v3.2.1/nrf/dts/bindings/sensor/pixart,pmw3360.yaml`

### PMW3389 (ZMK third-party driver)
Location: `/home/christian/ncs_projects/zmk-driver-pmw3389-main/`

- Implements the **Zephyr Input API** (`input_report_rel`) — *not* the Sensor API
- This is a **ZMK-specific driver** and is **incompatible** with the nRF Desktop `motion_sensor.c` module
- No SROM download needed (PMW3389 has built-in firmware)
- CPI range: 50–16000 in steps of 50 (9-bit register pair RESOLUTION_H:L)
- Product ID: `0x47`
- Supports motion burst, IRQ-driven or polling modes, rotation, invert axes
- Used as reference for register addresses, timing constants, and the initialization sequence

### Key Differences PMW3360 → PMW3389

| Feature | PMW3360 | PMW3389 |
|---|---|---|
| Product ID | `0x42` | `0x47` |
| SROM firmware | Required (4 KB blob) | Not needed |
| CPI range | 100–12000 (step 100) | 50–16000 (step 50) |
| CPI register | `CONFIG1` (0x0F, 8-bit) | `RESOLUTION_L/H` (0x0E/0x0F, 9-bit) |
| API for nRF Desktop | Sensor API ✅ | Input API only (ZMK) ✅ used as reference |

---

## Actions Taken

The new driver is written from scratch to match the **nRF Desktop Sensor API** interface while using the PMW3389 register map and timing from both the ZMK driver and the PMW3389 datasheet. No existing driver files were modified.

### Files Created

| File | Purpose |
|------|---------|
| `nrf_desktop-2026-2-5/drivers/sensor/pmw3389/pmw3389.c` | Main driver — full Sensor API implementation |
| `nrf_desktop-2026-2-5/drivers/sensor/pmw3389/Kconfig` | Kconfig symbol `PMW3389` for the driver |
| `nrf_desktop-2026-2-5/dts/bindings/sensor/pixart,pmw3389.yaml` | DTS binding for `compatible = "pixart,pmw3389"` |
| `nrf_desktop-2026-2-5/include/sensor/pmw3389.h` | Public header — `PMW3389_ATTR_CPI`, `PMW3389_ATTR_REST_ENABLE` |

### Files Modified

| File | Change |
|------|--------|
| `nrf_desktop-2026-2-5/CMakeLists.txt` | Added `include/` to include path; added `pmw3389.c` via `zephyr_library_sources_ifdef` |
| `nrf_desktop-2026-2-5/Kconfig` | Added `drivers/sensor/pmw3389/Kconfig` rsource |
| `nrf_desktop-2026-2-5/src/hw_interface/Kconfig.motion` | Added `DESKTOP_MOTION_SENSOR_PMW3389_ENABLE` choice option and type string |
| `nrf_desktop-2026-2-5/configuration/common/motion_sensor.h` | Added `#elif CONFIG_DESKTOP_MOTION_SENSOR_PMW3389_ENABLE` block with compatible string and attribute map |
| `nrf_desktop-2026-2-5/configuration/nice_nano_nrf52840/app.overlay` | Replaced disabled `pmw3360` node with active `pmw3389@0` node on `&spi1` |
| `nrf_desktop-2026-2-5/configuration/nice_nano_nrf52840/prj.conf` | Replaced `PMW3360_ENABLE=n` with `PMW3389_ENABLE=y`, enabled CPI=1600 and stack size |

---

## Driver Architecture

The new driver (`pmw3389.c`) mirrors the PMW3360 driver structure so it integrates cleanly with `motion_sensor.c`:

- **Async init state machine** – 2 steps: `POWER_UP` (CS toggle + reset write) → `CONFIGURE` (50 ms later: read product ID, set CPI, enable REST modes). No SROM loading needed.
- **Motion burst read** – Reads 6 bytes from `MOTION_BURST` (0x50) register: `[motion, observation, dx_l, dx_h, dy_l, dy_h]`. Only re-sends the burst register address when leaving burst mode.
- **IRQ handling** – MOTION pin (P1.13) fires `GPIO_INT_LEVEL_ACTIVE`; ISR disables interrupt and schedules a work item that calls the registered `sensor_trigger_handler_t`.
- **CPI** – 9-bit value: `reg_val = (cpi / 50) - 1`. High bit in `RESOLUTION_H` (0x0F), low 8 bits in `RESOLUTION_L` (0x0E).
- **REST modes** – Controlled via bit 5 of `CONFIG2` (0x10).
- **SPI** – Mode 3 (CPOL=1 CPHA=1), MSB first, 2 MHz initial frequency, CS managed manually with datasheet-correct inter-frame delays.

---

## Hardware Pins (nice!nano)

| Signal | Pin | GPIO |
|--------|-----|------|
| NRESET | D9 | P1.06 |
| MISO | D10 | P0.09 |
| MOSI | D16 | P0.10 |
| SCLK | D14 | P1.11 |
| MOTION | D15 | P1.13 |
| NCS | D18 | P1.15 |

The SPI pin assignments are already defined in `boards/nicekeyboards/nice_nano/nice_nano-pinctrl.dtsi` (`spi1_default` / `spi1_sleep`). The overlay only needed to add the CS GPIO and the sensor device node

## Register Map

Address | Register | Access | Default Value 
|----------|---------|-------------|-------------|
0x00 | Product_ID | R | 0x47 
0x01 | Revision_ID | R | 0x01 
0x02 | Motion | RW | 0x20 
0x03 | Delta_X_L | R | 0x00 
0x04 | Delta_X_H | R | 0x00 
0x05 | Delta_Y_L | R | 0x00 
0x06 | Delta_Y_H | R | 0x00 
0x07 | SQUAL | R | 0x00 
0x08 | RawData_Sum | R | 0x00 
0x09 | Maximum_RawData | R | 0x00 
0x0A | Minimum_RawData | R | 0x00 
0x0B | Shutter_Lower | R | 0x12 
0x0C | Shutter_Upper | R | 0x00 
0x0D | Ripple Control | RW | 0x07 
0x0E | Resolution_L | RW | 0x00 
0x0F | Resolution_H | RW | 0x42 
0x10 | Config2 | RW | 0x20
0x11 | Angle_Tune | RW | 0x00 
0x12 | Frame_Capture | RW | 0x00 
0x13 | SROM_Enable | W | N/A 
0x14 | Run_Downshift | RW | 0x32 
0x15 | Rest1_Rate_Lower | RW | 0x00 
0x16 | Rest1_Rate_Upper | RW | 0x00 
0x17 | Rest1_Downshift | RW | 0x1F 
0x18 | Rest2_Rate_Lower | RW | 0x63
0x19 | Rest2_Rate_Upper | RW | 0x00
0x1A | Rest2_Downshift | RW | 0xBC
0x1B | Rest3_Rate_Lower | RW | 0xF3
0x1C | Rest3_Rate_Upper | RW | 0x01
0x24 | Observation | RW | 0x00
0x25 | Data_Out_Lower | R | 0x00
0x26 | Data_Out_Upper | R | 0x00
0x2A | SROM_ID | R | 0x00
0x2B | Min_SQ_Run | RW | 0x10
0x2C | RawData_Threshold | RW | 0x0A
0x2D | Control2 | RW | 0x00
0x2E | Config5_L | RW | 0x00
0x2F | Config5_H | RW | 0x00
0X3A | Power_Up_Reset | W | N/A
0x3B | Shutdown | W | N/A
0x3F | Inverse_Product_ID | R | 0XB9
0x41 | LiftCutoff_Cal3 | RW | 0x00
0x42 | Angle_Snap | RW | 0x00
0x4A | LiftCutoff_Cal1 | RW | 0x00
0x50 | Motion_Burst | RW | 0x00
0x62 | SROM_Load_Burst | W | N/A
0x63 | Lift_Config | RW | 0x02
0x64 | RawData_Burst | R | 0x00
0x65 | LiftCutoff_Cal2 | R | 0x00
0x71 | LiftCutoff_Cal_Timeout | RW | 0x27
0x72 | LiftCutoff_Cal_Min_Length | RW | 0x09
0x73 | PWM_Period_Cnt | RW | 0x00
0x74 | PWM_Width_Cnt | RW | 0x00
