# nRF Desktop Project Reference

This document provides a concise overview of the project structure and configuration, focusing on the `nrf_desktop_5` setup for the nice!nano v2 board.

## Key Concepts

The project has two main configuration layers:

1.  **Board Definition (`boards/`):** Defines the *hardware* itself to the Zephyr RTOS. This layer describes the microcontroller, its available peripherals (GPIO, SPI, I2C), and the board's default settings. These files are reusable for any project targeting this board.
2.  **Application Configuration (`nrf_desktop_5/configuration/`):** Defines how the *nRF Desktop application* runs on a specific board. It enables software features and maps them to the physical hardware defined in the board layer.

---

## Application Configuration (`nrf_desktop_5/configuration/nice_nano_v2_nrf52840/`)

This directory configures the `nrf_desktop_5` application to run on the `nice_nano_v2`.

### `prj.conf`
- **Purpose:** Enables/disables software modules for the application. It's the main switchboard for features.
- **Current Setup:**
    - Enables USB & BLE.
    - Sets the device type to **Mouse**.
    - Enables `MOTION_BUTTONS`, allowing GPIO buttons to simulate mouse movement for testing.
    - Maps logical button IDs to mouse clicks and movement directions.

### `app.overlay`
- **Purpose:** Maps the software features from `prj.conf` to physical hardware pins. It acts as a "wiring diagram" for the application.
- **Current Setup:**
    - Defines a `gpio-keys` node.
    - Maps the logical buttons (for movement and clicks) to the specific GPIO pins on the nice!nano that will be used. The pin numbers correspond to the `arduino_pro_micro_pins.dtsi` file in the board definition.

### `pm_static.yml`
- **Purpose:** Defines the flash memory layout.
- **Importance:** This is **critical** for the nice!nano's UF2 bootloader. It ensures the application code starts at address `0x1000`, which is where the bootloader expects to find it. It also reserves space for the application to store settings.

---

## Board Definition (`boards/aliexpress/nice_nano_v2/`)

This directory defines the nice!nano v2 hardware for the Zephyr build system.

### `board.yml`
- **Purpose:** The main entry point for Zephyr's v2 board system.
- **Details:** Identifies the board as `nice_nano_v2` and specifies it uses the `nrf52840_qiaa` SoC.

### `nice_nano_v2_nrf52840.dts`
- **Purpose:** The master hardware blueprint for the board.
- **Details:** Describes all peripherals available on the nRF52840 chip (GPIO controllers, SPI/I2C buses, etc.).

### `nice_nano_v2_nrf52840_defconfig`
- **Purpose:** Sets the default build configuration for the board.
- **Details:** Any project built for this board inherits these settings. The most important setting is `CONFIG_BUILD_OUTPUT_UF2=y`, which tells the compiler to generate the `.uf2` file needed for flashing.

### `arduino_pro_micro_pins.dtsi`
- **Purpose:** Provides a convenient mapping between the nRF52840's physical GPIO pins and the standard "Pro Micro" pin names (e.g., `D0`, `D1`, `D2`).
- **Importance:** Allows you to use readable pin names in the `app.overlay` instead of having to look up GPIO port/pin numbers.

---

## Future Project Scenarios

### How to Build a Keyboard

1.  **Modify `prj.conf`:**
    - Change `CONFIG_DESKTOP_PERIPHERAL_TYPE_MOUSE=y` to `CONFIG_DESKTOP_PERIPHERAL_TYPE_KEYBOARD=y`.
    - Disable mouse-specific features: `# CONFIG_DESKTOP_MOTION_BUTTONS_ENABLE is not set`
    - Enable keyboard matrix scanning: `CONFIG_DESKTOP_HID_KEYMAP_ENABLE=y` and `CONFIG_ZMK_KSCAN=y`.

2.  **Modify `app.overlay`:**
    - Remove the `gpio-keys` node.
    - Add a keyboard matrix definition. This requires defining nodes for your rows and columns and then creating a `kscan` node to tie them together.

    ```dts
    / {
        chosen {
            zmk,kscan = &kscan0;
        };

        kscan0: kscan_0 {
            compatible = "zmk,kscan-gpio-matrix";
            diode-direction = "col2row";
            row-gpios
                = <&gpio0 20 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>
                , <&gpio0 17 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>;
            col-gpios
                = <&gpio0 24 GPIO_ACTIVE_HIGH>
                , <&gpio0 22 GPIO_ACTIVE_HIGH>;
        };
    };
    ```

3.  **Add Keymap Definition:**
    - Create a `nice_nano_v2_nrf52840.keymap` file in the application configuration directory. This file defines the layout of your keyboard and what each key press sends.
    - Set `CONFIG_DESKTOP_HID_KEYMAP_FILE="configuration/nice_nano_v2_nrf52840/nice_nano_v2_nrf52840.keymap"` in `prj.conf`.

### How to Use a Real Motion Sensor (e.g., PMW3360)

1.  **Modify `prj.conf`:**
    - Disable motion buttons: `# CONFIG_DESKTOP_MOTION_BUTTONS_ENABLE is not set`.
    - Enable the driver for your specific sensor, for example: `CONFIG_DESKTOP_MOTION_SENSOR_PMW3360_ENABLE=y`.
    - Configure sensor properties like CPI: `CONFIG_DESKTOP_MOTION_SENSOR_CPI=1600`.

2.  **Modify `app.overlay`:**
    - Make sure the SPI bus the sensor is on is enabled. For the nice!nano, this is `&spi1`.
    - Add a node for the sensor on the SPI bus, specifying its chip select and interrupt pins.

    ```dts
    &spi1 {
        status = "okay";

        pmw3360: pmw3360@0 {
            compatible = "pixart,pmw3360";
            reg = <0>;
            spi-max-frequency = <2000000>;
            cs-gpios = <&gpio0 9 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;  /* D10/A10 */
            irq-gpios = <&gpio1 4 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>; /* D8/A8 */
        };
    };
    ```

## Build Fixes and Configuration Steps

This section details the specific modifications made to resolve build errors and configure the project for simulated mouse functionality on the nice!nano v2.

### 1. `nrf_desktop_5/CMakeLists.txt` Modifications

The original `CMakeLists.txt` caused conflicts between the Kconfig system and the C preprocessor regarding include paths.

*   **Initial Problem:** The `APPLICATION_CONFIG_DIR` variable, when used with `zephyr_include_directories`, led to Kconfig path duplication errors (`File not found: .../configuration/nice_nano_v2_nrf52840/configuration/nice_nano_v2_nrf52840/prj.conf`).
*   **Solution:** The `set(APPLICATION_CONFIG_DIR ...)` line was commented out, and the board-specific configuration path was directly added to `include_directories` along with `configuration/common`. This ensures all necessary header files are found without confusing the Kconfig system.

    ```cmake
    # Original:
    # set(APPLICATION_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/configuration/\${NORMALIZED_BOARD_TARGET}")
    # ...
    # zephyr_include_directories(
    #   configuration/common
    #   ${APPLICATION_CONFIG_DIR}
    #   )

    # Modified:
    include_directories(
      "configuration/common"
      "${CMAKE_CURRENT_LIST_DIR}/configuration/nice_nano_v2_nrf52840"
      )
    ```

### 2. `boards/aliexpress/nice_nano_v2/Kconfig.defconfig` Correction

A Kconfig dependency loop was identified.

*   **Problem:** The `if BOARD_NICE_NANO_V2` and `endif` block in `Kconfig.defconfig` incorrectly made `SOC_NRF52840_QIAA` dependent on `BOARD_NICE_NANO_V2`, while the board also depended on the SoC.
*   **Solution:** The `if/endif` block was removed, allowing `Kconfig.defconfig` to simply set default values without creating circular dependencies.

### 3. `nrf_desktop_5/configuration/nice_nano_v2_nrf52840/prj.conf` Updates

Several Kconfig symbols were outdated or undefined, leading to build warnings treated as errors.

*   **Problem 1:** `CONFIG_DESKTOP_BLE_ENABLE=y` was an undefined symbol.
*   **Solution 1:** Replaced with `CONFIG_DESKTOP_BT=y`, which is the current Kconfig symbol for Bluetooth support.
*   **Problem 2:** `CONFIG_DESKTOP_HID_BUTTON_MOUSE_0_KEY_ID` and `CONFIG_DESKTOP_HID_BUTTON_MOUSE_1_KEY_ID` were undefined.
*   **Solution 2:** These lines were removed as the button-to-HID mapping is now handled via `hid_keymap_def.h`.
*   **Problem 3:** `CONFIG_SETTINGS_NVS_POINTER=y` was undefined.
*   **Solution 3:** This line was removed as it is an obsolete configuration option.
*   **Problem 4:** `BUILD_ASSERT(CONFIG_BT_ID_MAX >= ...)` failed due to insufficient Bluetooth identities.
*   **Solution 4:** Added `CONFIG_BT_ID_MAX=4` to ensure enough identities for bonding scenarios.

### 4. Creation of Missing Header Files

Several header files were missing from the board-specific configuration directory, causing "file not found" errors.

*   **`port_state_def.h`**: Created an empty file based on the `nrf52840dk_nrf52840` template, adapted for `gpio0` only.
*   **`hid_keymap_def.h`**: Created this file to define the mapping from button IDs to HID mouse report usages.
    *   `key_id = 4` mapped to `usage_id = 1` (Left Click).
    *   `key_id = 5` mapped to `usage_id = 2` (Right Click).
*   **`hid_keyboard_leds_def.h`**: Created an empty file, as keyboard LED functionality is not required for a mouse.
*   **`buttons_def.h`**: Created this file to map specific GPIO pins to logical button IDs for simulated movement and clicks.
    *   Pins 17, 20, 22, 24 mapped to buttons 0-3 for movement.
    *   Pins 6, 8 mapped to buttons 4-5 for clicks.
*   **`settings_loader_def.h`**: Created an empty file to satisfy a build requirement, as this file is optional but unconditionally included by some SDK components.

### 5. `nrf_desktop_5/configuration/nice_nano_v2_nrf52840/app.overlay` Creation

*   **Purpose:** Defined device tree nodes for the physical buttons.
*   **Configuration:** Mapped `button_17`, `button_20`, `button_22`, `button_24`, `button_6`, and `button_8` to their respective GPIO pins on `gpio0` with pull-up resistors and active-low logic.

These changes collectively addressed the various build issues and configured the project for the requested simulated mouse functionality.
