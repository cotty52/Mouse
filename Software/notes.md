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
