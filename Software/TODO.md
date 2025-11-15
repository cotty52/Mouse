# Project TODO List

This document is for tracking current tasks, next steps, and completed items to help organize your workflow.

## Current Task

*   **Integrate the PMW3389 motion sensor.**

## Next Steps

*   **Download the PMW3389 driver manually.**
*   **Provide the chip select (CS) and interrupt (IRQ) pins for the sensor.**
*   Update `app.overlay` with the correct sensor pins.
*   Build and test the firmware with the real motion sensor.
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
