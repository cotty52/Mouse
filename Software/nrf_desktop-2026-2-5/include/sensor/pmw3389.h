/*
 * Copyright (c) 2026 Christian
 * PMW3389 optical mouse sensor driver - public API header
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ZEPHYR_INCLUDE_PMW3389_H_
#define ZEPHYR_INCLUDE_PMW3389_H_

/**
 * @file pmw3389.h
 * @brief Header file for the PMW3389 optical mouse sensor driver.
 *
 * Implements the Zephyr sensor API so the driver is compatible with
 * the nRF Desktop motion_sensor module.
 */

#include <zephyr/drivers/sensor.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Sensor-specific attributes for the PMW3389. */
enum pmw3389_attribute {
	/** CPI value for both X and Y axes.
	 *  Range: 50 – 16000, must be a multiple of 50.
	 *  Pass as sensor_value.val1.
	 */
	PMW3389_ATTR_CPI = SENSOR_ATTR_PRIV_START,

	/** Enable (1) or disable (0) the sensor's built-in Power-Save (REST) modes.
	 *  Pass as sensor_value.val1 (boolean).
	 */
	PMW3389_ATTR_REST_ENABLE,
};

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_PMW3389_H_ */
