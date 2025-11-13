# Pin Configuration Guide for nice!nano Mouse

This guide explains how to configure GPIO pins for buttons, motion, and other hardware features in the nRF Desktop application.

---

## Overview

The nRF Desktop application uses two main methods for pin configuration:

1. **buttons_def.h** - Defines which GPIO pins are used for physical buttons
2. **Kconfig settings** - Maps button IDs to functions (motion, left click, right click, etc.)
3. **Device Tree (DTS)** - Configures hardware peripherals like sensors, LEDs, etc.

---

## 1. Physical Button Pin Configuration

**File**: `nrf_desktop_3/configuration/nice_nano_v2_nrf52840/buttons_def.h`

### Current Configuration

```c
#include <caf/gpio_pins.h>

const struct {} buttons_def_include_once;

static const struct gpio_pin col[] = {};

static const struct gpio_pin row[] = {
	{ .port = 0, .pin = 2 },   // Button ID 0
	{ .port = 1, .pin = 15 },  // Button ID 1
	{ .port = 1, .pin = 14 },  // Button ID 2
	{ .port = 0, .pin = 29 },  // Button ID 3
	{ .port = 0, .pin = 31 },  // Button ID 4
	{ .port = 0, .pin = 24 },  // Button ID 5
	{ .port = 0, .pin = 22 },  // Button ID 6
	{ .port = 0, .pin = 4 },   // Button ID 7
};
```

### nice!nano v2 GPIO Pin Reference

```
Arduino Pin → nRF52840 Pin → Port.Pin
-------------------------------------
D2  → P0.10 → { .port = 0, .pin = 10 }
D3  → P0.09 → { .port = 0, .pin = 9 }
D4  → P1.13 → { .port = 1, .pin = 13 }
D5  → P1.11 → { .port = 1, .pin = 11 }
D6  → P0.07 → { .port = 0, .pin = 7 }
D7  → P0.05 → { .port = 0, .pin = 5 }
D8  → P0.04 → { .port = 0, .pin = 4 }
D9  → P0.26 → { .port = 0, .pin = 26 }

A0  → P0.02 → { .port = 0, .pin = 2 }
A1  → P0.29 → { .port = 0, .pin = 29 }
A2  → P0.31 → { .port = 0, .pin = 31 }
A3  → P0.30 → { .port = 0, .pin = 30 }

MOSI → P0.45 → { .port = 0, .pin = 45 }
MISO → P0.43 → { .port = 0, .pin = 43 }
SCK  → P0.10 → { .port = 0, .pin = 10 }

SDA → P0.17 → { .port = 0, .pin = 17 }
SCL → P0.20 → { .port = 0, .pin = 20 }
```

### How to Change Button Pins

**Example**: Configure pins for a typical mouse layout

```c
static const struct gpio_pin row[] = {
	{ .port = 0, .pin = 10 },  // Button ID 0 - Left Click (D2)
	{ .port = 0, .pin = 9 },   // Button ID 1 - Right Click (D3)
	{ .port = 1, .pin = 13 },  // Button ID 2 - Middle Click (D4)
	{ .port = 1, .pin = 11 },  // Button ID 3 - Forward (D5)
	{ .port = 0, .pin = 7 },   // Button ID 4 - Back (D6)
};
```

**Important Notes**:
- Button IDs are assigned sequentially (0, 1, 2, 3...)
- The array index determines the button ID
- Buttons are active LOW with internal pull-up by default
- Empty `col[]` array means simple GPIO buttons (not a matrix)

---

## 2. Button Function Mapping (Motion & Clicks)

**File**: `nrf_desktop_3/configuration/nice_nano_v2_nrf52840/prj.conf`

### Motion Buttons (Temporary - for testing without optical sensor)

To use buttons to simulate mouse movement:

```properties
# Enable motion button feature
CONFIG_DESKTOP_MOTION_BUTTONS_ENABLE=y

# Map button IDs to movement directions
CONFIG_DESKTOP_MOTION_BUTTONS_UP_KEY_ID=0
CONFIG_DESKTOP_MOTION_BUTTONS_DOWN_KEY_ID=1
CONFIG_DESKTOP_MOTION_BUTTONS_LEFT_KEY_ID=2
CONFIG_DESKTOP_MOTION_BUTTONS_RIGHT_KEY_ID=3

# Movement speed (pixels per second when button held)
CONFIG_DESKTOP_MOTION_BUTTONS_MOTION_PER_SEC=400
```

**How it works**:
- When button ID 0 is pressed → cursor moves up
- When button ID 1 is pressed → cursor moves down
- When button ID 2 is pressed → cursor moves left
- When button ID 3 is pressed → cursor moves right

### Mouse Click Buttons

Mouse click mapping is handled in `hid_keymap_def.h` or via the HID provider module.

**File**: `nrf_desktop_3/configuration/nice_nano_v2_nrf52840/hid_keymap_def.h`

Look for structures that map button IDs to HID mouse buttons:

```c
// Example mapping (you may need to create this if not present)
static const struct {
	uint16_t key_id;
	uint16_t hid_usage;
} mouse_button_map[] = {
	{0, 0x01},  // Button ID 0 → Left Click
	{1, 0x02},  // Button ID 1 → Right Click  
	{2, 0x04},  // Button ID 2 → Middle Click
	{3, 0x08},  // Button ID 3 → Forward
	{4, 0x10},  // Button ID 4 → Back
};
```

**HID Mouse Button Codes**:
```
0x01 = Left Button
0x02 = Right Button
0x04 = Middle Button
0x08 = Button 4 (Forward)
0x10 = Button 5 (Back)
```

---

## 3. Complete Example Configurations

### Configuration A: Testing with Motion Buttons (No Sensor)

**buttons_def.h**:
```c
static const struct gpio_pin row[] = {
	{ .port = 0, .pin = 10 },  // ID 0 - Up motion
	{ .port = 0, .pin = 9 },   // ID 1 - Down motion
	{ .port = 1, .pin = 13 },  // ID 2 - Left motion
	{ .port = 1, .pin = 11 },  // ID 3 - Right motion
	{ .port = 0, .pin = 7 },   // ID 4 - Left click
	{ .port = 0, .pin = 5 },   // ID 5 - Right click
};
```

**prj.conf**:
```properties
# Enable buttons
CONFIG_CAF_BUTTONS=y
CONFIG_CAF_BUTTONS_POLARITY_INVERSED=y

# Enable motion buttons (IDs 0-3 for movement)
CONFIG_DESKTOP_MOTION_BUTTONS_ENABLE=y
CONFIG_DESKTOP_MOTION_BUTTONS_UP_KEY_ID=0
CONFIG_DESKTOP_MOTION_BUTTONS_DOWN_KEY_ID=1
CONFIG_DESKTOP_MOTION_BUTTONS_LEFT_KEY_ID=2
CONFIG_DESKTOP_MOTION_BUTTONS_RIGHT_KEY_ID=3
CONFIG_DESKTOP_MOTION_BUTTONS_MOTION_PER_SEC=400

# Click detector for buttons 4-5
CONFIG_CAF_CLICK_DETECTOR=y
```

### Configuration B: Production Mouse (With PMW3360 Sensor)

**buttons_def.h**:
```c
static const struct gpio_pin row[] = {
	{ .port = 0, .pin = 10 },  // ID 0 - Left click (D2)
	{ .port = 0, .pin = 9 },   // ID 1 - Right click (D3)
	{ .port = 1, .pin = 13 },  // ID 2 - Middle click (D4)
	{ .port = 1, .pin = 11 },  // ID 3 - Forward (D5)
	{ .port = 0, .pin = 7 },   // ID 4 - Back (D6)
	{ .port = 0, .pin = 5 },   // ID 5 - DPI switch (D7)
};
```

**prj.conf**:
```properties
# Enable buttons
CONFIG_CAF_BUTTONS=y
CONFIG_CAF_BUTTONS_POLARITY_INVERSED=y

# Enable PMW3360 motion sensor (disable motion buttons)
# CONFIG_DESKTOP_MOTION_BUTTONS_ENABLE=n
CONFIG_DESKTOP_MOTION_SENSOR_PMW3360_ENABLE=y
CONFIG_DESKTOP_MOTION_SENSOR_CPI=1600

# Click detector for mouse buttons
CONFIG_CAF_CLICK_DETECTOR=y
```

---

## 4. Device Tree (DTS) Pin Configuration

**File**: `nrf_desktop_3/configuration/nice_nano_v2_nrf52840/app.overlay`

### Adding Button Nodes (Alternative Method)

You can also define buttons in device tree instead of `buttons_def.h`:

```dts
/ {
	buttons {
		compatible = "gpio-keys";
		
		button0: button_0 {
			gpios = <&gpio0 10 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
			label = "Left Click";
		};
		
		button1: button_1 {
			gpios = <&gpio0 9 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
			label = "Right Click";
		};
		
		button2: button_2 {
			gpios = <&gpio1 13 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
			label = "Middle Click";
		};
	};
};
```

**Note**: The nRF Desktop application primarily uses `buttons_def.h`, but DTS is useful for documenting hardware.

---

## 5. Step-by-Step: Configure Your Mouse Buttons

### Step 1: Decide Your Layout

Example for a 5-button mouse:
- Left Click → Button ID 0
- Right Click → Button ID 1  
- Middle Click (wheel button) → Button ID 2
- Forward (side button) → Button ID 3
- Back (side button) → Button ID 4

### Step 2: Choose GPIO Pins

Select available pins on nice!nano:
```
Left Click   → D2  (P0.10)
Right Click  → D3  (P0.09)
Middle Click → D4  (P1.13)
Forward      → D5  (P1.11)
Back         → D6  (P0.07)
```

### Step 3: Edit buttons_def.h

```c
#include <caf/gpio_pins.h>

const struct {} buttons_def_include_once;

static const struct gpio_pin col[] = {};

static const struct gpio_pin row[] = {
	{ .port = 0, .pin = 10 },  // Button ID 0 - Left Click (D2)
	{ .port = 0, .pin = 9 },   // Button ID 1 - Right Click (D3)
	{ .port = 1, .pin = 13 },  // Button ID 2 - Middle Click (D4)
	{ .port = 1, .pin = 11 },  // Button ID 3 - Forward (D5)
	{ .port = 0, .pin = 7 },   // Button ID 4 - Back (D6)
};
```

### Step 4: Enable Buttons in prj.conf

```properties
CONFIG_CAF_BUTTONS=y
CONFIG_CAF_BUTTONS_POLARITY_INVERSED=y
CONFIG_CAF_BUTTONS_PM_KEEP_ALIVE=n
CONFIG_CAF_CLICK_DETECTOR=y
```

### Step 5: Rebuild and Test

```bash
cd nrf_desktop_3
west build -b nice_nano_v2/nrf52840 --pristine
# Flash the new zephyr.uf2
```

---

## 6. Testing Your Configuration

### Test Motion Buttons

1. Enable motion buttons in `prj.conf`
2. Define which button IDs control movement
3. Build and flash
4. Press buttons to see cursor move

### Test Click Buttons

1. Wire buttons between GPIO pins and GND
2. Build and flash
3. Open Windows Mouse Properties → Hardware → Properties
4. Click buttons and watch HID events in event log

### Verify Button IDs

Add debug logging to see which button IDs are triggered:

**prj.conf**:
```properties
CONFIG_DESKTOP_BUTTONS_LOG_LEVEL_DBG=y
CONFIG_CAF_BUTTONS_LOG_LEVEL_DBG=y
```

Check logs via UART or RTT to see: `Button X pressed/released`

---

## 7. Common Pin Conflicts to Avoid

### nice!nano Reserved Pins

```
P0.06 (TX)  - UART debug (can disable)
P0.08 (RX)  - UART debug (can disable)
P0.15       - Blue LED (can share)
P0.18       - Reset (do not use)
P0.21       - Reset (do not use)
```

### Pins Used by Peripherals

**If using SPI sensor (PMW3360)**:
- P0.45 (MOSI)
- P0.43 (MISO)  
- P0.10 (SCK) ⚠️ Shared with D2
- P0.?? (CS) - Choose available GPIO

**If using I2C**:
- P0.17 (SDA)
- P0.20 (SCL)

**If using QDEC (wheel encoder)**:
- P0.02 (Phase A)
- P0.03 (Phase B)

**If using PWM LEDs** (current config):
- P0.26, P0.27, P0.28 (PWM0)
- P0.11, P0.12, P0.13 (PWM1)

---

## 8. Recommended Pin Allocation (Custom PCB)

### Suggested Layout

```
Function          | Pin    | Port.Pin
------------------+--------+----------
Left Click        | Custom | P0.10
Right Click       | Custom | P0.09  
Middle Click      | Custom | P1.13
Forward Button    | Custom | P1.11
Back Button       | Custom | P0.07
DPI Switch        | Custom | P0.05

PMW3360 CS        | Custom | P0.04
PMW3360 MOSI      | Fixed  | P0.45
PMW3360 MISO      | Fixed  | P0.43
PMW3360 SCK       | Fixed  | P0.10 (conflict!) → Use P0.44

Encoder A         | Custom | P0.02
Encoder B         | Custom | P0.03

Status LED R      | Custom | P0.26 (PWM0-0)
Status LED G      | Custom | P0.27 (PWM0-1)
Status LED B      | Custom | P0.28 (PWM0-2)

Battery ADC       | Custom | P0.30 (AIN6)
Battery Enable    | Custom | P0.06
Charge Status     | Custom | P0.14

USB D+            | Fixed  | (Internal)
USB D-            | Fixed  | (Internal)
```

---

## 9. Quick Reference Commands

```bash
# Edit button pins
nano nrf_desktop_3/configuration/nice_nano_v2_nrf52840/buttons_def.h

# Edit button functions
nano nrf_desktop_3/configuration/nice_nano_v2_nrf52840/prj.conf

# Build with changes
cd nrf_desktop_3
west build -b nice_nano_v2/nrf52840

# Clean rebuild
west build -b nice_nano_v2/nrf52840 --pristine

# Check button configuration
grep "CONFIG.*BUTTON" build/zephyr/.config | grep "=y"
```

---

## Summary

- **Physical pins**: Configure in `buttons_def.h` using port/pin numbers
- **Button functions**: Configure in `prj.conf` using Kconfig options
- **Motion buttons**: Use `CONFIG_DESKTOP_MOTION_BUTTONS_*_KEY_ID` to map button IDs
- **Click buttons**: Automatically mapped by HID provider based on button ID order
- **Always rebuild** with `--pristine` after changing button definitions

For your custom mouse, start with motion buttons for testing, then migrate to a real PMW3360 sensor once hardware is ready!
