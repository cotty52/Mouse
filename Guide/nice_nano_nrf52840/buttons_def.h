/*
 * Copyright (c) 2026 Christian
 *
 * SPDX-License-Identifier: MIT
 */

#include <caf/gpio_pins.h>

/* This configuration file is included only once from button module and holds
 * information about pins forming keyboard matrix.
 */

/* This structure enforces the header file is included only once in the build.
 * Violating this requirement triggers a multiple definition error at link time.
 */
const struct {} buttons_def_include_once;

/* No column scanning - direct button connections */
static const struct gpio_pin col[] = {};

/* Button GPIO pins - indices correspond to KEY_ID(row, col) values
 * Order MUST exactly match button definitions in app.overlay */
static const struct gpio_pin row[] = {
	{ .port = 0, .pin = 8 },  /* button0: Left Click (D0 = P0.08) */
	{ .port = 0, .pin = 31 }, /* button1: Right Click (D21 = P0.31) */
	{ .port = 0, .pin = 29 }, /* button2: Middle Click (D20 = P0.29) */
	{ .port = 0, .pin = 17 }, /* button3: Thumb 1 (D2 = P0.17) */
	{ .port = 0, .pin = 20 }, /* button4: Thumb 2 (D3 = P0.20) */
	{ .port = 0, .pin = 2 },  /* button5: Extra Button (D19 = P0.02) */
};
