# GEMINI.md - nRF Desktop Mouse Project

## Project Overview

This project aims to build a custom wireless mouse using the **nRF52840 nice!nano v2** microcontroller. The software is based on Nordic Semiconductor's **nRF Desktop** sample application, which runs on the Zephyr RTOS.

The goal is to create a working HID mouse with USB and Bluetooth LE connectivity, tailored for the nice!nano v2 hardware. The `nrf_desktop_5` directory is a clean copy of the nRF Desktop sample and will be the foundation for the new, properly structured build.

**Key Technologies:**

*   **CPU:** nRF52840 (on nice!nano v2)
*   **OS:** Zephyr RTOS
*   **Connectivity:** Bluetooth Low Energy (BLE), USB
*   **Build System:** CMake / west
*   **Bootloader:** Adafruit UF2 Bootloader

**Architecture:**

The application uses a modular, event-driven architecture. An `app_event_manager` dispatches events to various modules that handle HID reporting, BLE communication, USB state, and other features. The project is highly configurable via Kconfig.

## Hardware and Bootloader

The **nice!nano v2** board uses an **Adafruit UF2 bootloader**. This is a key consideration for the build process:

*   **Bootloader Variant:** The `nosd` (no SoftDevice) variant is used.
*   **Flash Layout:** The application code must be configured to start at memory address `0x1000` to be compatible with the bootloader.
*   **Flashing Method:** The board is flashed by dragging and dropping a `.uf2` file onto the `NICENANO` drive when the board is in bootloader mode.

## Building and Running

The following instructions are based on the working configuration from the `test_project/nrf_desktop_3` directory.

**Prerequisites:**

*   Zephyr RTOS development environment and nRF Connect SDK are set up.
*   A custom board definition for `nice_nano_v2` is available (likely in the `boards/` directory).

**Build Command:**

To build the application for the nice!nano v2, run the following command from within the `nrf_desktop_5` directory:

```bash
west build -b nice_nano_v2/nrf52840 --pristine
```

**Flashing Instructions:**

1.  Connect the nice!nano v2 to your computer.
2.  **Double-tap the reset button** on the board to enter bootloader mode. A `NICENANO` drive should appear.
3.  Drag and drop the `build/zephyr/zephyr.uf2` file from your project directory onto the `NICENANO` drive.
4.  The board will automatically reboot and should enumerate as an HID-compliant mouse.

**Verification:**

You can verify the UF2 file's target address using the provided Python script:

```bash
python reference/uf2conv.py --info build/zephyr/zephyr.uf2
# The output should confirm: Target Address is 0x00001000
```

## Development Conventions

*   **Modularity:** The codebase is highly modular, with each module responsible for a specific feature. Modules are located in the `src/modules` directory.
*   **Event-Driven:** The application uses an event-driven architecture. Events are defined in the `src/events` directory and handled by the appropriate modules.
*   **Configuration:** Project features and modules are configured using Kconfig.
*   **Coding Style:** The code follows the Zephyr coding style.