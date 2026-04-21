/*
 * Copyright (c) 2018-2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
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
 * HID keymap for nRF52840 DK mouse debugging.
 *
 * DK button0 (P0.11) → KEY_ID(0,0) = 0x0000: BLE peer control (click_detector)
 * DK button1 (P0.12) → KEY_ID(0,1) = 0x0001: Mouse Button 1 (Left Click)
 * DK button2 (P0.24) → KEY_ID(0,2) = 0x0002: Mouse Button 2 (Right Click)
 * DK button3 (P0.25) → KEY_ID(0,3) = 0x0003: Mouse Button 3 (Middle Click)
 */
static const struct hid_keymap hid_keymap[] = {
	{ KEY_ID(0, 1), 1, REPORT_ID_MOUSE },  /* button1: Left Click */
	{ KEY_ID(0, 2), 2, REPORT_ID_MOUSE },  /* button2: Right Click */
	{ KEY_ID(0, 3), 3, REPORT_ID_MOUSE },  /* button3: Middle Click */
};
