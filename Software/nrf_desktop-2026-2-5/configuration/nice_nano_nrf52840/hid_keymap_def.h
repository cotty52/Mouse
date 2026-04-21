/*
 * Copyright (c) 2026 Christian
 *
 * SPDX-License-Identifier: MIT
 */

#include "hid_keymap.h"
#include <caf/key_id.h>

/* This configuration file is included only once from hid_state module and holds
 * information about mapping between buttons and generated reports.
 */

/* This structure enforces the header file is included only once in the build.
 * Violating this requirement triggers a multiple definition error at link time.
 */
const struct {} hid_keymap_def_include_once;

/*
 * HID keymap. The Consumer Control keys are defined in section 15 of
 * the HID Usage Tables document under the following URL:
 * https://www.usb.org/sites/default/files/hut1_12.pdf
 */
static const struct hid_keymap hid_keymap[] = {
	{ KEY_ID(0, 0), 1, REPORT_ID_MOUSE },     /* button0 P0.08: Button 1 - Left Click */
	{ KEY_ID(0, 1), 2, REPORT_ID_MOUSE },     /* button1 P0.31: Button 2 - Right Click */
	{ KEY_ID(0, 2), 3, REPORT_ID_MOUSE },     /* button2 P0.29: Button 3 - Middle Click */
	{ KEY_ID(0, 3), 4, REPORT_ID_MOUSE },     /* button3 P0.17: Button 4 - Back */
	{ KEY_ID(0, 4), 5, REPORT_ID_MOUSE },     /* button4 P0.20: Button 5 - Forward */
	/* button5: KEY_ID(0, 5) = 0x0005 - used for BLE peer control (click_detector) */
};
