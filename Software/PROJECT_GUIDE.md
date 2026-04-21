# nRF Desktop Custom Wireless Mouse Project Guide

**Last Updated:** February 20, 2026  
**Project Status:** Hardware complete, BLE working, integrating PMW3389 sensor driver

---

## Project Overview

This project is a custom wireless mouse based on Nordic Semiconductor's **nRF Desktop** reference application. The nRF Desktop framework is an event-driven, modular application built on Zephyr RTOS and Nordic's Common Application Framework (CAF), designed for HID peripherals like mice, keyboards, and dongles.

**Hardware:** The mouse uses a **nice!nano v1/v2** development board featuring the nRF52840 SoC (1MB Flash, 256KB RAM, Bluetooth 5.0). A custom PCB routes connections between the nice!nano and peripheral components including a **PMW3389** optical sensor, multiple buttons, a scroll wheel encoder, and mode selection switches.

**Goal:** To develop production-quality firmware for a fully-featured wireless mouse supporting Bluetooth connectivity, multiple button inputs, high-precision optical tracking, scroll wheel functionality, and battery management. The firmware leverages the nRF Desktop's proven architecture while customizing it for this specific hardware configuration.

**Current Status:** Bluetooth HID is working on the nice!nano. USB HID works on both nice!nano and nRF52840 DK. Dongle pairing is working with the DK but not yet with the nice!nano (peer control button key ID needs fixing). Currently integrating the PMW3389 optical sensor driver.

---

## Task Tracking

### Current To-Do

- [ ] **Integrate PMW3389 driver** - add third-party driver files to build system, wire up DTS/Kconfig, build successfully
- [ ] **Test PMW3389 on hardware** - verify SPI communication, motion events generated correctly
- [ ] **Fix dongle pairing with nice!nano** - set `CONFIG_DESKTOP_BLE_PEER_CONTROL_BUTTON` to a valid key_id (check RTT log for actual key_ids), erase dongle bonds and re-pair
- [ ] Complete app.overlay configuration for all PCB connections
- [ ] Test scroll wheel encoder functionality (QDEC configuration)
- [ ] Configure battery monitoring for power management
- [ ] Generate UF2 firmware file for nice!nano bootloader
- [ ] Implement mode switch logic (2.4GHz vs Bluetooth)

### Completed

- [x] Build and flash firmware to nRF52840 DK for debugging
- [x] Verified button events working via RTT logging
- [x] USB HID working on DK (J3 port) and nice!nano
- [x] Bluetooth HID working on nice!nano
- [x] Flashed nRF52840 Dongle using DK as J-Link programmer (required full erase due to SoftDevice/B0 bootloader incompatibility)
- [x] Dongle pairing working with DK as mouse
- [x] Found PMW3389 third-party driver source files, placed in project for reference

### Future Enhancements

- Battery level indicator via LED
- Multiple device pairing/switching (peer selector)
- DPI switching functionality
- Power optimization for extended battery life
- USB connectivity support
- Custom LED effects for different modes
- Firmware update mechanism (DFU over Bluetooth)

---

## Quick Start for AI Agents

### Typical Workflow

1. **Understanding Changes:** Start by reviewing this guide and [NOTES.md](NOTES.md) for technical references
2. **Locating Files:** Use the "Critical Files & Directories" section below to find relevant configuration files
3. **Making Changes:** Modify the appropriate combination of DTS overlays, Kconfig settings, and _def.h header files
4. **Building:** Reference the build command in [NOTES.md](NOTES.md); builds are done via nRF Connect extension or west CLI
5. **Verification:** Check build output in `build_custom_*/` directories for errors and generated artifacts

### Build Command (from NOTES.md)

```bash
cd /home/christian/ncs/v3.2.1 && west build --build-dir /home/christian/ncs_projects/nrf_desktop-2026-2-5/build_custom_2 /home/christian/ncs_projects/nrf_desktop-2026-2-5
```

### Common Modification Patterns

- **Hardware pins changed?** → Update `app.overlay` DTS nodes + corresponding `*_def.h` files
- **Need to enable/disable features?** → Modify `prj.conf` Kconfig options
- **Button behavior change?** → Edit `buttons_def.h` and possibly HID keymap
- **LED effects?** → Update `led_state_def.h` effect arrays
- **Sensor parameters?** → Check DTS properties and Kconfig sensor settings

### Configuration Hierarchy

The nRF Desktop uses a three-layer configuration approach:

1. **Device Tree (.dts/.overlay)** - Hardware description: pins, peripherals, physical connections
2. **Kconfig (prj.conf)** - Feature selection: which modules to enable, stack sizes, protocol options
3. **Definition Headers (*_def.h)** - Board-specific mappings: GPIO arrays, lookup tables, effect definitions

Changes often require coordinating across all three layers for consistency.

---

## Hardware Configuration

### nice!nano Board Specifications

- **MCU:** Nordic nRF52840 (ARM Cortex-M4F @ 64 MHz)
- **Flash:** 1 MB
- **RAM:** 256 KB
- **Bluetooth:** 5.0 with 2 Mbps PHY
- **USB:** USB 2.0 Full Speed device
- **RTC:** 32.768 kHz crystal oscillator on-board
- **Form Factor:** Arduino Pro Micro compatible pinout
- **Bootloader:** Adafruit nRF52 bootloader (UF2 support)

### Custom PCB Pinout

| Component | Pin Label | GPIO | Notes |
|-----------|-----------|------|-------|
| Left Button | D0 | P0.08 | Primary click |
| Right Button | D21 | P0.31 | Secondary click |
| Thumb Button 1 | D2 | P0.17 | Side button (forward) |
| Thumb Button 2 | D3 | P0.20 | Side button (back) |
| Extra Button | D19 | P0.02 | Additional control |
| Scroll Wheel Button | D20 | P0.29 | Middle click |
| Encoder Phase A | D5 | P0.22 | Scroll wheel quadrature |
| Encoder Phase B | D6 | P1.00 | Scroll wheel quadrature |
| 2.4 GHz Switch | D7 | P0.11 | Mode selector (future) |
| Bluetooth Switch | D8 | P1.04 | Mode selector |
| **PMW3389 Sensor** | | | |
| NRESET (sensor) | D9 | P1.06 | Sensor reset pin |
| MISO (sensor) | D10 | P0.09 | SPI MISO |
| MOSI (sensor) | D16 | P0.10 | SPI MOSI |
| SCLK (sensor) | D14 | P1.11 | SPI Clock |
| MOTION (sensor) | D15 | P1.13 | IRQ/Motion detect |
| NCS (sensor) | D18 | P1.15 | SPI Chip Select |

### Sensor Information

- **Model:** PMW3389 high-performance optical gaming sensor
- **Interface:** SPI
- **Note:** Currently using PMW3360 driver files from nRF Desktop examples as starting point (may require adaptation)
- **Features:** Adjustable CPI/DPI, high tracking performance, motion detection interrupt

### User Controls

- **7 Buttons:** Left, Right, 2x Thumb, Extra, Scroll Wheel Click, Mode Select
- **Scroll Wheel:** Rotary encoder with detents (quadrature output)
- **Mode Switches:** 2.4GHz and Bluetooth selector switches (hardware present, software TBD)

---

## Critical Files & Directories

### Board Definition Files

**Location:** [boards/nicekeyboards/nice_nano/](boards/nicekeyboards/nice_nano/)

| File | Purpose |
|------|---------|
| [nice_nano.dts](boards/nicekeyboards/nice_nano/nice_nano.dts) | Base hardware description (nRF52840 chip, flash layout, peripherals) |
| [nice_nano-pinctrl.dtsi](boards/nicekeyboards/nice_nano/nice_nano-pinctrl.dtsi) | Pin controller definitions for peripheral routing |
| [arduino_pro_micro_pins.dtsi](boards/nicekeyboards/nice_nano/arduino_pro_micro_pins.dtsi) | Arduino Pro Micro pin compatibility layer |
| [nice_nano.yaml](boards/nicekeyboards/nice_nano/nice_nano.yaml) | Board metadata and supported features |
| [Kconfig.nice_nano](boards/nicekeyboards/nice_nano/Kconfig.nice_nano) | Board-specific Kconfig symbols |
| nice_nano_1_0_0.overlay / nice_nano_2_0_0.overlay | Hardware version-specific DTS overlays |

### Application Configuration (Primary Work Area)

**For nice!nano v1:** [nrf_desktop-2026-2-5/configuration/nice_nano_nrf52840/](nrf_desktop-2026-2-5/configuration/nice_nano_nrf52840/)  
**For nice!nano v2:** [nrf_desktop-2026-2-5/configuration/nice_nano_v2_nrf52840/](nrf_desktop-2026-2-5/configuration/nice_nano_v2_nrf52840/)

| File | Purpose |
|------|---------|
| **prj.conf** | Main Kconfig configuration (feature enables, module selection, stack sizes) |
| **app.overlay** | Device tree overlay for application-specific hardware (buttons, sensor, LEDs) |
| **buttons_def.h** | Button GPIO pin mapping array (must match DTS button order) |
| **battery_def.h** | Battery measurement configuration (ADC channel, voltage divider, SoC table) |
| **led_state_def.h** | LED effect definitions per application state |
| **port_state_def.h** | Port state configuration (USB/Bluetooth switching logic) |
| **hid_keymap_def.h** | HID keyboard key mappings (if keyboard functionality used) |
| **hid_keyboard_leds_def.h** | Keyboard LED indicators configuration |
| **click_detector_def.h** | Click gesture detection parameters |
| **pm_static.yml** | Partition Manager static configuration (flash memory layout) |
| **sysbuild.conf** | Sysbuild configuration for bootloader integration |

### Common/Shared Definitions

**Location:** [nrf_desktop-2026-2-5/configuration/common/](nrf_desktop-2026-2-5/configuration/common/)

| File | Purpose |
|------|---------|
| [hid_report_desc.c/h](nrf_desktop-2026-2-5/configuration/common/hid_report_desc.c) | HID report descriptors (USB/BLE) - defines mouse/keyboard reports |
| [led_state.h](nrf_desktop-2026-2-5/configuration/common/led_state.h) | LED state enumeration used across all boards |
| [motion_sensor.h](nrf_desktop-2026-2-5/configuration/common/motion_sensor.h) | Motion sensor interface definitions |
| [fn_key_id.h](nrf_desktop-2026-2-5/configuration/common/fn_key_id.h) | Function key identifiers |
| dev_descr.h | USB device descriptor definitions |

### Application Source Code

**Location:** [nrf_desktop-2026-2-5/src/](nrf_desktop-2026-2-5/src/)

| Directory | Purpose |
|-----------|---------|
| [events/](nrf_desktop-2026-2-5/src/events/) | Event type definitions (motion_event, button_event, hid_event, battery_event, etc.) |
| [hw_interface/](nrf_desktop-2026-2-5/src/hw_interface/) | Hardware abstraction modules (motion_sensor.c, battery_meas.c, wheel.c, etc.) |
| [modules/](nrf_desktop-2026-2-5/src/modules/) | Application logic modules (hid_state.c, ble_adv_ctrl.c, ble_bond.c, led_state.c, etc.) |
| [util/](nrf_desktop-2026-2-5/src/util/) | Utility functions (config_channel_transport.c, hid_keymap.c, dfu_lock.c) |
| [main.c](nrf_desktop-2026-2-5/src/main.c) | Application entry point (minimal - just initializes event manager) |

### Build Output Directories

**Locations:** 
- [nrf_desktop-2026-2-5/build_custom_1/](nrf_desktop-2026-2-5/build_custom_1/)
- [nrf_desktop-2026-2-5/build_custom_2/](nrf_desktop-2026-2-5/build_custom_2/)
- [nrf_desktop-2026-2-5/build_dk/](nrf_desktop-2026-2-5/build_dk/), [build_dongle/](nrf_desktop-2026-2-5/build_dongle/), [build_gmouse/](nrf_desktop-2026-2-5/build_gmouse/) (reference builds)

| Artifact | Purpose |
|----------|---------|
| zephyr/zephyr.hex | Final firmware hex file (ready for programming) |
| zephyr/zephyr.elf | ELF file with debug symbols |
| zephyr/zephyr.dts | Final resolved device tree (after all overlays applied) |
| compile_commands.json | For IDE code completion and analysis |
| CMakeCache.txt | CMake configuration cache |
| pm.config | Partition Manager configuration output |
| partitions.yml | Memory partition definitions |

### Reference Configurations

**Full-Featured Mouse Reference:** [nrf_desktop-2026-2-5/configuration/nrf52840gmouse_nrf52840/](nrf_desktop-2026-2-5/configuration/nrf52840gmouse_nrf52840/)

This is the most complete mouse configuration in nRF Desktop, featuring:
- PMW3360 motion sensor (similar to PMW3389)
- Multiple buttons and scroll wheel
- RGB LED effects
- Battery charging and monitoring
- Peer selector (multi-device pairing)
- Both USB and Bluetooth

Use this as a reference when implementing advanced features or troubleshooting.

---

## Architecture Overview

### Event-Driven Model

The nRF Desktop application uses an **event-driven architecture** coordinated by the **Application Event Manager**:

1. **Hardware Interface Modules** ([hw_interface/](nrf_desktop-2026-2-5/src/hw_interface/)) generate events from physical hardware:
   - `motion_sensor.c` → `motion_event` (dx, dy, sensor data)
   - `battery_meas.c` → `battery_event` (voltage, state of charge)
   - CAF `buttons` module → `button_event` (key ID, pressed/released)
   - `wheel.c` → `wheel_event` (scroll delta)

2. **Application Modules** ([modules/](nrf_desktop-2026-2-5/src/modules/)) process events and manage state:
   - `hid_state.c` - Central HID report manager, collects input events and generates HID reports
   - `ble_adv_ctrl.c` - Controls Bluetooth advertising
   - `ble_bond.c` - Manages Bluetooth bonding and pairing
   - `led_state.c` - Visual feedback via LEDs based on system state
   - `usb_state.c` - USB connection management

3. **Output Modules** send HID reports to host devices:
   - BLE HID service
   - USB HID class
   - Config channel for runtime configuration

**Key Principle:** Modules are loosely coupled. They subscribe to events they care about and emit events when state changes. No direct module-to-module calls.

### Three-Layer Configuration System

The nRF Desktop separates configuration concerns into three distinct layers:

#### Layer 1: Device Tree (Hardware)

**Files:** `.dts`, `.dtsi`, `.overlay`  
**Purpose:** Describe physical hardware - what peripherals exist, which GPIO pins are connected, bus configurations

Example:
```dts
&spi1 {
    pmw3360@0 {
        compatible = "pixart,pmw3360";
        reg = <0>;
        spi-max-frequency = <2000000>;
        irq-gpios = <&gpio0 11 GPIO_ACTIVE_LOW>;
        reset-gpios = <&gpio1 6 GPIO_ACTIVE_LOW>;
    };
};
```

#### Layer 2: Kconfig (Features)

**Files:** `prj.conf`, `Kconfig`, `Kconfig.*`  
**Purpose:** Select which features and modules to compile, set stack sizes, configure protocol parameters

Example:
```properties
CONFIG_DESKTOP_ROLE_HID_PERIPHERAL=y
CONFIG_DESKTOP_PERIPHERAL_TYPE_MOUSE=y
CONFIG_DESKTOP_MOTION_SENSOR_PMW3360_ENABLE=y
CONFIG_DESKTOP_MOTION_SENSOR_CPI=1600
CONFIG_CAF_BUTTONS=y
CONFIG_CAF_LEDS=y
```

#### Layer 3: Definition Headers (Board Mapping)

**Files:** `*_def.h` in configuration directories  
**Purpose:** Map logical components to physical implementation, define lookup tables and effect arrays

Example (buttons_def.h):
```c
static const struct gpio_pin row[] = {
    { .port = 0, .pin = 8 },   // Left button (button0 in DTS)
    { .port = 0, .pin = 31 },  // Right button (button1 in DTS)
    { .port = 0, .pin = 17 },  // Thumb 1 (button2 in DTS)
    // Order must match DTS button node order!
};
```

### Key Modules Reference

| Module | Location | Purpose |
|--------|----------|---------|
| **motion_sensor** | [hw_interface/motion_sensor.c](nrf_desktop-2026-2-5/src/hw_interface/motion_sensor.c) | Interfaces with optical sensor (PMW3360/3389), generates motion events with dx/dy |
| **hid_state** | [modules/hid_state.c](nrf_desktop-2026-2-5/src/modules/hid_state.c) | Central HID manager, combines button/motion/wheel events into HID reports |
| **buttons** | CAF framework | Scans GPIO buttons, generates button_event (provided by framework, configured via _def.h) |
| **wheel** | [hw_interface/wheel.c](nrf_desktop-2026-2-5/src/hw_interface/wheel.c) | QDEC-based scroll wheel support, generates wheel_event |
| **battery_meas** | [hw_interface/battery_meas.c](nrf_desktop-2026-2-5/src/hw_interface/battery_meas.c) | ADC-based battery monitoring, generates battery_event |
| **ble_bond** | [modules/ble_bond.c](nrf_desktop-2026-2-5/src/modules/ble_bond.c) | Bluetooth pairing and bond management |
| **led_state** | [modules/led_state.c](nrf_desktop-2026-2-5/src/modules/led_state.c) | State machine for LED visual feedback |
| **config_channel** | [modules/config_channel.c](nrf_desktop-2026-2-5/src/modules/config_channel.c) | Runtime configuration over HID feature reports |

### Build System

- **west:** Nordic's meta-tool for managing Zephyr RTOS projects
- **CMake:** Build system generator, processes [CMakeLists.txt](nrf_desktop-2026-2-5/CMakeLists.txt)
- **sysbuild:** Multi-image build system for bootloader + application
- **Partition Manager:** Nordic's flash memory partition management

Build flow: `west build` → CMake configures → ninja/make compiles → artifacts in `build_*/zephyr/`

---

## Common Tasks & Module Guide

### Modifying Button Configuration

**Required Changes:**

1. **Update Device Tree** ([app.overlay](nrf_desktop-2026-2-5/configuration/nice_nano_nrf52840/app.overlay)):
   ```dts
   / {
       buttons {
           compatible = "gpio-keys";
           button0: button_0 {
               gpios = <&gpio0 8 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
               label = "Left Click";
               zephyr,code = <INPUT_BTN_0>;
           };
           // Add more buttons...
       };
       
       aliases {
           sw0 = &button0;
           sw1 = &button1;
           // System uses aliases to find buttons
       };
   };
   ```

2. **Update Definition Header** ([buttons_def.h](nrf_desktop-2026-2-5/configuration/nice_nano_nrf52840/buttons_def.h)):
   ```c
   static const struct gpio_pin row[] = {
       { .port = 0, .pin = 8 },   // button0 - order MUST match DTS!
       { .port = 0, .pin = 31 },  // button1
       { .port = 0, .pin = 17 },  // button2
   };
   ```

3. **Enable in Kconfig** (prj.conf):
   ```
   CONFIG_CAF_BUTTONS=y
   CONFIG_CAF_BUTTONS_POLARITY_INVERSED=y  # If using pull-up with active-low
   ```

**Critical:** The order in `buttons_def.h` row[] array must exactly match the button node order in the DTS. The index becomes the button's key_id in button_event.

### Motion Sensor Configuration

**Required Changes:**

1. **Device Tree SPI Node** (app.overlay):
   ```dts
   &spi1 {
       compatible = "nordic,nrf-spim";
       status = "okay";
       cs-gpios = <&gpio1 15 GPIO_ACTIVE_LOW>;  // NCS pin
       
       pmw3360: pmw3360@0 {
           compatible = "pixart,pmw3360";  // May work for PMW3389
           reg = <0>;
           spi-max-frequency = <2000000>;
           irq-gpios = <&gpio1 13 GPIO_ACTIVE_LOW>;    // MOTION pin
           reset-gpios = <&gpio1 6 GPIO_ACTIVE_LOW>;   // NRESET pin
       };
   };
   ```

2. **Pin Controller Configuration** (app.overlay):
   ```dts
   &pinctrl {
       spi1_default: spi1_default {
           group1 {
               psels = <NRF_PSEL(SPIM_SCK, 1, 11)>,   // SCLK
                       <NRF_PSEL(SPIM_MOSI, 0, 10)>,  // MOSI
                       <NRF_PSEL(SPIM_MISO, 0, 9)>;   // MISO
           };
       };
       
       spi1_sleep: spi1_sleep {
           group1 {
               psels = <NRF_PSEL(SPIM_SCK, 1, 11)>,
                       <NRF_PSEL(SPIM_MOSI, 0, 10)>,
                       <NRF_PSEL(SPIM_MISO, 0, 9)>;
               low-power-enable;
           };
       };
   };
   ```

3. **Kconfig Selection** (prj.conf):
   ```
   CONFIG_DESKTOP_MOTION_SENSOR_PMW3360_ENABLE=y
   CONFIG_DESKTOP_MOTION_SENSOR_CPI=1600
   CONFIG_DESKTOP_MOTION_SENSOR_CPI_LIST="400 800 1600 3200"
   CONFIG_DESKTOP_MOTION_SENSOR_THREAD_STACK_SIZE=1024
   ```

**Note for PMW3389:** May require driver modifications if PMW3360 driver doesn't fully support it. Check SPI command compatibility.

### Scroll Wheel / Encoder Configuration

**Required Changes:**

1. **Device Tree QDEC Node** (app.overlay):
   ```dts
   &qdec {
       status = "okay";
       enable-pin = <0xFFFFFFFF>;  // No enable pin
       led-pin = <0xFFFFFFFF>;     // No LED pin
       led-pre = <0>;
       steps = <24>;  // Pulses per rotation (adjust for your encoder)
       a-pin = <22>;  // P0.22 - Encoder A
       b-pin = <32>;  // P1.00 - Encoder B (32 = 32 + 0)
   };
   ```

2. **Enable Wheel Module** (prj.conf):
   ```
   CONFIG_DESKTOP_WHEEL_ENABLE=y
   CONFIG_DESKTOP_WHEEL_SENSOR_VALUE_DIVIDER=1
   CONFIG_DESKTOP_WHEEL_INVERT_AXIS=n
   ```

### LED Effect Configuration

**Definition Header** ([led_state_def.h](nrf_desktop-2026-2-5/configuration/nice_nano_nrf52840/led_state_def.h)):

```c
#include <led_effect.h>

// Define colors
#define LED_COLOR_BLUE LED_COLOR(0, 0, 255)
#define LED_COLOR_GREEN LED_COLOR(0, 255, 0)

// Define effects for each state
static const struct led_effect led_system_state_effect[LED_SYSTEM_STATE_COUNT] = {
    [LED_SYSTEM_STATE_IDLE] = LED_EFFECT_LED_BREATH(200, LED_COLOR_BLUE),
    [LED_SYSTEM_STATE_CHARGING] = LED_EFFECT_LED_BREATH(500, LED_COLOR_GREEN),
    [LED_SYSTEM_STATE_ERROR] = LED_EFFECT_LED_BLINK(100, LED_COLOR(255, 0, 0)),
};
```

**Device Tree for LED** (app.overlay):
```dts
/ {
    leds {
        compatible = "gpio-leds";
        led0: led_0 {
            gpios = <&gpio0 15 GPIO_ACTIVE_HIGH>;
            label = "Blue LED";
        };
    };
};

&pwm0 {
    status = "okay";
    pinctrl-0 = <&pwm0_default>;
    pinctrl-names = "default";
};
```

### Battery Monitoring Configuration

1. **Device Tree ADC** (app.overlay):
   ```dts
   / {
       battery-charger {
           compatible = "gpio-leds";  // Reusing LED driver
           battery_charging_led: led_1 {
               gpios = <&gpio0 13 GPIO_ACTIVE_HIGH>;
           };
       };
   };
   
   &adc {
       status = "okay";
       #address-cells = <1>;
       #size-cells = <0>;
       
       channel@7 {
           reg = <7>;  // AIN7
           zephyr,gain = "ADC_GAIN_1_6";
           zephyr,reference = "ADC_REF_INTERNAL";
           zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
           zephyr,input-positive = <NRF_SAADC_AIN7>;
           zephyr,resolution = <12>;
       };
   };
   ```

2. **Definition Header** ([battery_def.h](nrf_desktop-2026-2-5/configuration/nice_nano_nrf52840/battery_def.h)):
   ```c
   #include <caf/battery_def.h>
   
   static const struct battery_config {
       .adc_channel = 7,  // AIN7
       .voltage_divider = {
           .upper = 1000,  // Upper resistor in kΩ
           .lower = 1000,  // Lower resistor in kΩ
       },
   };
   
   // Voltage to State of Charge lookup table
   static const uint16_t battery_voltage_mv[] = {3000, 3300, 3600, 3700, 3800, 4100};
   static const uint8_t battery_soc_pct[] = {0, 10, 30, 50, 80, 100};
   ```

### HID Report Modification

**Common HID Descriptor** ([hid_report_desc.c](nrf_desktop-2026-2-5/configuration/common/hid_report_desc.c)):

This file defines the HID report structure sent to the host. Mouse reports typically include:
- Buttons (bitmap)
- X/Y motion (signed 16-bit)
- Wheel (signed 8-bit)
- Optional: extra buttons, horizontal wheel, etc.

To add buttons or change report structure, modify this file and ensure `hid_state.c` fills the report correctly.

### Adding a New Module

1. **Create source file** in [src/modules/](nrf_desktop-2026-2-5/src/modules/)
2. **Add Kconfig** options in `src/modules/Kconfig`
3. **Register event listeners** using `EVENT_LISTENER()` and `EVENT_SUBSCRIBE()` macros
4. **Update CMakeLists.txt** in modules directory
5. **Enable in prj.conf** with your new CONFIG option

---

## Common Issues & Troubleshooting

### Build Failures

**Symptom:** CMake configuration fails or compilation errors

**Solutions:**
- **Clean build:** Delete entire `build_*` directory and rebuild from scratch
- **Board name mismatch:** Verify board name exactly matches directory under `boards/` (e.g., `nice_nano_nrf52840`)
- **Missing dependencies:** Check `CMakeCache.txt` for ZEPHYR_BASE and NCS paths
- **Kconfig conflicts:** Look for warnings about disabled options, check dependencies in Kconfig files
- **Out of memory:** Check CMake errors for RAM/Flash overflow → disable modules or adjust partition sizes

### Motion Sensor Not Working

**Symptom:** No mouse movement, motion events not generated

**Diagnostics:**
- **Check SPI communication:** Verify MISO/MOSI/SCK pins in pinctrl match PCB
- **IRQ/Motion pin:** Confirm `irq-gpios` in DTS matches MOTION pin, check polarity (ACTIVE_LOW vs ACTIVE_HIGH)
- **Chip select:** Verify `cs-gpios` matches NCS pin
- **Reset pin:** Confirm `reset-gpios` matches NRESET pin, check polarity
- **Driver compatibility:** PMW3389 may need driver adjustments if using PMW3360 driver
- **SPI frequency:** Try lower frequency (500kHz) if communication unstable
- **Device binding:** Check `compatible = "pixart,pmw3360"` string matches driver

**Debugging:**
```
CONFIG_LOG=y
CONFIG_DESKTOP_MOTION_LOG_LEVEL_DBG=y
CONFIG_SPI_LOG_LEVEL_DBG=y
```

### Button Mapping Incorrect

**Symptom:** Wrong button triggers action, or buttons don't work

**Fixes:**
- **Order mismatch:** Ensure `buttons_def.h` row[] array order exactly matches DTS button node order
- **GPIO incorrect:** Double-check port and pin numbers against PCB schematic
- **Polarity:** Verify GPIO_ACTIVE_LOW/HIGH matches hardware (pull-up usually needs ACTIVE_LOW)
- **Missing alias:** Check that `sw0`, `sw1`, etc. aliases are defined in DTS
- **Array size:** Confirm `row[]` array has correct number of entries

**Debugging:**
```
CONFIG_CAF_BUTTONS_LOG_LEVEL_DBG=y
```

### Flash Memory Won't Fit

**Symptom:** Linker error about region 'FLASH' overflowed

**Solutions:**
- **Adjust partitions:** Edit `pm_static.yml` to increase app partition size
- **Disable unused modules:** Comment out unnecessary `CONFIG_*` options in prj.conf
- **Use release build:** Release builds have optimizations and less debug info
- **Reduce logging:** Set `CONFIG_LOG=n` or reduce log levels
- **Check bootloader size:** If using MCUboot/B0, verify bootloader partition isn't too large

### UF2 Firmware Generation

**Command for nice!nano** (from [NOTES.md](NOTES.md)):

```bash
# For nRF52840 (nice!nano)
uf2conv.py firmware.hex -c -f 0xADA52840

# Or if using .bin file (specify app start address)
uf2conv.py firmware.bin -c -b 0x1000 -f 0xADA52840
```

**Process:**
1. Build project → generates `zephyr/zephyr.hex`
2. Convert to UF2 using uf2conv.py script
3. Double-tap reset button on nice!nano to enter bootloader mode
4. Copy .uf2 file to NICENANO USB drive that appears
5. Bootloader flashes firmware and resets automatically

**Adafruit Bootloader Family IDs:**
- nRF52840: `0xADA52840`
- nRF52833: `0x621E937A`
- Bootloader itself: `0xd663823c`

### Bluetooth Pairing Issues

**Symptom:** Can't pair device, or paired device won't reconnect

**Solutions:**
- **Clear bonds:** Many configs have button combo to clear bonds (check `ble_bond` module config)
- **Peer limit:** Default may support only 1-2 peers, increase if needed:
  ```
  CONFIG_BT_MAX_PAIRED=4
  CONFIG_CAF_BLE_BOND_PEER_ERASE_PRESS_TIME_MS=10000
  ```
- **Advertising timeout:** Increase advertising time if host slow to scan:
  ```
  CONFIG_DESKTOP_BLE_ADVERTISING_DURATION=300
  ```
- **Check host side:** Remove device from host Bluetooth settings and re-pair
- **Reset settings:** Erase settings partition (may require reflashing)

**Debugging:**
```
CONFIG_BT_DEBUG_LOG=y
CONFIG_DESKTOP_BLE_LOG_LEVEL_DBG=y
```

### Scroll Wheel Not Working

**Symptom:** Scroll wheel doesn't generate events

**Checks:**
- **QDEC pins:** Verify `a-pin` and `b-pin` match encoder connections (note: P1.x pins are 32 + x)
- **Steps configuration:** Adjust `steps` in QDEC node to match encoder pulses per rotation
- **Divider:** Tune `CONFIG_DESKTOP_WHEEL_SENSOR_VALUE_DIVIDER` for sensitivity
- **Invert axis:** Try `CONFIG_DESKTOP_WHEEL_INVERT_AXIS=y` if scrolling backwards
- **Enable QDEC:** Ensure `CONFIG_DESKTOP_WHEEL_ENABLE=y` in prj.conf

### Power Consumption / Battery Life

**Optimizations:**
- Enable low-power modes in prj.conf
- Reduce advertising interval when not connected
- Use connection parameter updates for longer intervals
- Implement sleep mode with wakeup on button press
- Disable unnecessary peripherals (USB if not used)
- Use motion sensor sleep modes

---

## Reference Documentation

### nRF Desktop Documentation

| Document | Location | Purpose |
|----------|----------|---------|
| Main README | [README.rst](nrf_desktop-2026-2-5/README.rst) | Project overview, features, requirements |
| Integration Guide | [integration.rst](nrf_desktop-2026-2-5/integration.rst) | Step-by-step porting to new hardware |
| Board Configuration | [board_configuration.rst](nrf_desktop-2026-2-5/board_configuration.rst) | How to configure board support |
| Architecture | [description.rst](nrf_desktop-2026-2-5/description.rst) | Event flow, module relationships |
| Kconfig Options | [application_kconfig.rst](nrf_desktop-2026-2-5/application_kconfig.rst) | All application Kconfig symbols explained |
| Module Reference | [modules.rst](nrf_desktop-2026-2-5/modules.rst) | Index of all modules with links |
| Bluetooth | [bluetooth.rst](nrf_desktop-2026-2-5/bluetooth.rst) | BLE configuration and features |
| Bootloader/DFU | [bootloader_dfu.rst](nrf_desktop-2026-2-5/bootloader_dfu.rst) | Firmware update mechanisms |

### Module-Specific Documentation

Location: [nrf_desktop-2026-2-5/doc/](nrf_desktop-2026-2-5/doc/)

Each module has a dedicated .rst file with:
- Event table (what events the module produces and consumes)
- Configuration options
- Implementation details
- Usage examples

Key modules to understand:
- `motion_sensor.rst` - Motion sensor integration
- `hid_state.rst` - HID report generation
- `buttons.rst` - Button handling (CAF framework)
- `wheel.rst` - Scroll wheel support
- `battery_meas.rst` - Battery monitoring
- `ble_bond.rst` - Bluetooth pairing management

### Project-Specific References

- **[NOTES.md](NOTES.md)** - User-maintained technical reference with build commands, pinout, bootloader info
- **Hardware datasheets:**
  - nRF52840 Product Specification (Nordic Semiconductor)
  - PMW3389 Datasheet (PixArt Imaging)
  - nice!nano schematic and pinout

### External Documentation

- **Zephyr RTOS:** https://docs.zephyrproject.org/
  - Device Tree bindings reference
  - API documentation
  - Board porting guide
- **Nordic nRF Connect SDK:** https://docs.nordicsemi.com/
  - SDK documentation
  - Bluetooth LE guide
  - Power management
- **nice!nano:** https://nicekeyboards.com/docs/nice-nano/
  - Pinout diagrams
  - Bootloader documentation

---

## Workflow Notes

### Building Firmware

**Preferred Method:** nRF Connect for VS Code extension
- Use the extension's build/flash buttons
- Automatically handles board selection and configuration
- Integrated terminal shows build output

**Command Line Method:**
```bash
# Full build command (adjust build directory as needed)
west build --build-dir /home/christian/ncs_projects/nrf_desktop-2026-2-5/build_custom_2 \
           /home/christian/ncs_projects/nrf_desktop-2026-2-5

# Pristine build (clean + configure + compile)
west build -p always --build-dir ... /path/to/source

# Specify board and configuration
west build -b nice_nano_nrf52840 -- -DCONF_FILE=prj.conf
```

### Flashing Firmware to nice!nano

The nice!nano uses the Adafruit bootloader with UF2 support:

1. **Build firmware** → generates `build_*/zephyr/zephyr.hex`

2. **Convert to UF2:**
   ```bash
   # Install uf2conv if not available
   pip install adafruit-nrfutil
   
   # Convert (nRF52840 family ID)
   python uf2conv.py zephyr.hex -c -f 0xADA52840 -o firmware.uf2
   ```

3. **Enter bootloader mode:**
   - Double-tap the RESET button on nice!nano
   - A USB drive named "NICENANO" should appear

4. **Flash firmware:**
   - Copy `firmware.uf2` to the NICENANO drive
   - Bootloader automatically flashes and resets
   - Device starts running new firmware

**Troubleshooting:**
- If USB drive doesn't appear, try different USB cable or port
- Check that bootloader is installed (blue LED should pulse in bootloader mode)
- On Linux, may need udev rules for USB access

### Testing Configurations

Use separate build directories for experimenting:
- `build_custom_1/` - Stable configuration
- `build_custom_2/` - Experimental changes
- Keep build directories to avoid reconfiguring for each test

### Version Control Best Practices

**Track these directories/files:**
- `boards/nicekeyboards/nice_nano/` - Board definition changes
- `nrf_desktop-2026-2-5/configuration/nice_nano*/` - All application configs
- Any modifications to `nrf_desktop-2026-2-5/src/` (if customizing modules)
- This PROJECT_GUIDE.md and NOTES.md

**Don't track:**
- `build_*/` directories (build artifacts)
- `.cache/`, `.vscode/` (IDE files)
- `*.uf2`, `*.hex`, `*.bin` (generated firmware)

### Development Iteration Cycle

1. **Make configuration changes** (app.overlay, prj.conf, *_def.h)
2. **Build** via extension or west command
3. **Check for errors** in build output
4. **If successful, generate UF2** and flash to hardware
5. **Test functionality** with real inputs
6. **Iterate** based on results
7. **Document changes** in this PROJECT_GUIDE.md (update task tracking)

### Debugging Tips

- **Enable logging:** Add CONFIG_LOG=y and module-specific log levels in prj.conf
- **Use RTT:** Real-Time Transfer for log output without USB (requires debugger)
- **Check device tree:** Look at `build_*/zephyr/zephyr.dts` to see final resolved tree
- **Inspect events:** Enable event manager logging to see event flow
- **USB Serial:** Configure USB CDC ACM for serial console output (if USB available)

---

## Notes for AI Agents

### Quick Context Gathering

When joining this project, prioritize reading:
1. This **PROJECT_GUIDE.md** (you're reading it now)
2. **Task Tracking** section above for current work items
3. **[NOTES.md](NOTES.md)** for technical details and commands
4. **Hardware Configuration** section for pinout reference
5. Current **prj.conf** and **app.overlay** to see active configuration

### Making Effective Changes

- **Always check all three layers:** DTS, Kconfig, and _def.h files must be consistent
- **Reference examples:** Look at `nrf52840gmouse` configuration when unsure
- **Validate build:** Always build after changes to catch errors early
- **One feature at a time:** Test incrementally rather than changing everything at once
- **Document as you go:** Update task tracking when completing work

### Understanding User Intent

This project is about:
- ✅ Creating firmware for a custom wireless mouse
- ✅ Practical, working solutions for hardware integration
- ✅ Leveraging existing nRF Desktop modules where possible
- ❌ NOT about reinventing the framework or major architectural changes
- ❌ NOT about adding features that aren't needed for the mouse

### Communication Style

- Be direct and technical (user is engineering-focused)
- Explain *why* changes are needed, not just *what* to change
- Point to relevant files and line numbers
- When troubleshooting, provide multiple potential causes
- Offer alternatives when there are multiple valid approaches

---

**End of Guide**

*This document is a living reference - update the task tracking and any relevant sections as the project evolves.*
