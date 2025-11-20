# Project TODO List

This document is for tracking current tasks, next steps, and completed items to help organize your workflow.

## Current Task

*   **Set up and test Bluetooth connectivity.**

## Next Steps

*   Build the firmware using the **nRF Connect for VS Code extension** (manual build, not west commands).
*   Flash the updated firmware to the nice!nano v2 board.
*   Test Bluetooth pairing with a computer or mobile device.
*   Verify mouse functionality over Bluetooth (movement via buttons, left/right clicks).
*   Test switching between USB and Bluetooth modes.
*   Test Bluetooth bonding and reconnection.
*   Integrate the PMW3389 motion sensor (future task).
*   Design and 3D print a case for the mouse.
*   Implement power management for battery operation.

## Completed Tasks

*   Analyzed project documentation and ZMK reference.
*   Identified issues in previous board and application configurations.
*   Created a consolidated `nice_nano_v2` board definition in `boards/aliexpress/nice_nano_v2/`.
*   Corrected `board.yml` and `Kconfig` for the `nice_nano_v2` board.
*   Created application-specific configuration for `nrf_desktop_5/configuration/nice_nano_v2_nrf52840/`.
*   Added `pm_static.yml` for correct UF2 bootloader memory layout.
*   Created `prj.conf` to enable mouse functionality with simulated buttons.
*   Created `app.overlay` to map simulated buttons to GPIO pins.
*   Corrected `nrf_desktop_5/CMakeLists.txt` to include necessary directories.
*   Created `notes.md` with project setup and build fix details.
*   Built the nRF Desktop firmware for nice!nano v2.
*   Tested basic mouse functionality (movement, clicks) using the simulated buttons.
*   Documented the changes made to get the build to work in `notes.md`.
*   Configured Bluetooth settings in `prj.conf`:
    *   Enabled BLE peripheral configuration with proper device identification
    *   Added peer control and bonding support
    *   Configured Low Latency Packet Mode (LLPM) for reduced latency
    *   Set up proper connection parameters for HID mouse operation
    *   Configured dual HID subscriber support (USB + BLE)

## Build Instructions

**Note:** West workspace is not properly configured in this project. Use the **nRF Connect for VS Code extension** to build the firmware manually instead of west command-line tools.
