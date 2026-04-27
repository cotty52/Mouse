# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Custom wireless mouse firmware based on Nordic Semiconductor's **nRF Desktop** reference application, running on Zephyr RTOS. Hardware is a **nice!nano v1/v2** development board (nRF52840 SoC) connected to a custom PCB with a PMW3389 optical sensor, 6 buttons, and a scroll wheel encoder.

**Detailed documentation:** `Software/PROJECT_GUIDE.md` — read this first when joining the project. It contains task tracking, architecture deep-dives, common issues, and step-by-step guides for each hardware subsystem.

## Repository Structure

```
Mouse/
├── KiCad/            # PCB schematics and layout (KiCad project)
├── Documents/        # Datasheets, bootloader UF2, 3D models
├── Guide/
│   ├── README.md                   # New machine setup guide (start here after cloning)
│   ├── nice_nano/                  # Dated snapshot of board definition files
│   └── nice_nano_nrf52840/         # Dated snapshot of working config files
└── Software/
    ├── boards/                     # Zephyr board definitions
    │   ├── nicekeyboards/nice_nano/     # Official nice!nano port (nice_nano/nrf52840)
    │   └── aliexpress/nice_nano_v2/    # Aliexpress v2 (nice_nano_v2/nrf52840)
    ├── nrf_desktop-2026-04-23/     # ACTIVE project (most recent)
    │   ├── configuration/
    │   │   ├── nice_nano_nrf52840/ # Primary config files (edit here)
    │   │   ├── common/            # Shared HID descriptors
    │   │   └── nrf52840gmouse_nrf52840/ # Reference mouse config
    │   └── src/                   # Application source (events, modules, hw_interface)
    ├── nrf_desktop-2026-2-5/      # Older project snapshot (reference)
    └── reference/                 # ZMK nice!nano files (Zephyr reference)
```

**File naming convention:** All-uppercase `.md` files are for AI agent context. Lowercase files (e.g., `notes.md`) are human-written notes.

## Build System

**Primary method:** nRF Connect for VS Code extension (build/flash buttons in the sidebar). Open `Mouse.code-workspace` — this activates `nrf-connect.boardRoots` so the extension finds the custom `nice_nano` board definitions in `Software/boards/`. See `Guide/README.md` for full setup steps.

**CLI prerequisite:** Run `nrf-env` in fish shell first to activate the nRF Connect toolchain and make `west` available:
```fish
nrf-env   # sets PATH, ZEPHYR_BASE (/home/christian/ncs/v3.2.4/zephyr), toolchain vars
```
The `nrf-env` function is defined in `~/.config/fish/config.fish`. Toolchain: `/home/christian/ncs/toolchains/2ac5840438`.

**CLI build command** (copied from nRF Connect GUI output — canonical form):
```bash
west build \
  --build-dir /home/christian/Projects/Mouse/Software/nrf_desktop-2026-04-23/build_custom \
  /home/christian/Projects/Mouse/Software/nrf_desktop-2026-04-23 \
  --pristine \
  --board nice_nano/nrf52840 \
  --no-sysbuild \
  -- \
  -DBOARD_ROOT="/home/christian/Projects/Mouse/Software"
```

**Clean build:** Delete the `build_*/` directory entirely before rebuilding when changing board or CMake configuration.

**Build artifacts** appear in `build_*/zephyr/`:
- `zephyr.hex` — flash with J-Link or convert to UF2
- `zephyr.dts` — final resolved device tree (useful for debugging)

## Flashing (nice!nano UF2 Bootloader)

1. Build → generates `build_*/zephyr/zephyr.hex`
2. Convert to UF2: `uf2conv.py zephyr.hex -c -f 0xADA52840`
3. Double-tap RESET on nice!nano → `NICENANO` USB drive appears
4. Copy `.uf2` file to the drive; board auto-flashes and reboots

Family IDs: nRF52840 = `0xADA52840`, nRF52833 = `0x621E937A`, bootloader = `0xd663823c`

## Configuration Architecture

nRF Desktop uses three coordinated layers — changes often require touching all three:

| Layer | Files | Purpose |
|-------|-------|---------|
| **Device Tree** | `app.overlay`, `*.dts`, `*.dtsi` | Physical hardware: pins, peripherals, buses |
| **Kconfig** | `prj.conf` | Feature selection: which modules compile, stack sizes, BLE options |
| **Definition Headers** | `*_def.h` | Board mapping: GPIO arrays, LED effects, battery tables |

**Critical constraint:** The order of entries in `buttons_def.h` row[] must exactly match the button node order in `app.overlay`. The array index becomes the button's `key_id` in `button_event`.

## Active Configuration Files

All in `Software/nrf_desktop-2026-04-23/configuration/nice_nano_nrf52840/`:

| File | Role |
|------|------|
| `prj.conf` | Kconfig: enable/disable modules, set PMW3389, BLE params |
| `app.overlay` | DTS: SPI sensor, GPIO buttons, QDEC encoder, ADC battery |
| `buttons_def.h` | GPIO pin array for CAF buttons module |
| `led_state_def.h` | LED effect definitions per system state |
| `battery_def.h` | ADC channel, voltage divider, SoC lookup table |

## Custom PCB Pinout (nice!nano)

| Component | Arduino Label | GPIO |
|-----------|--------------|------|
| Left button | D0 | P0.08 |
| Thumb 1 | D2 | P0.17 |
| Thumb 2 | D3 | P0.20 |
| Encoder A | D5 | P0.22 |
| Encoder B | D6 | P1.00 |
| 2.4 GHz switch | D7 | P0.11 |
| Bluetooth switch | D8 | P1.04 |
| NRESET (sensor) | D9 | P1.06 |
| MISO (sensor) | D10 | P0.09 |
| MOSI (sensor) | D16 | P0.10 |
| SCLK (sensor) | D14 | P1.11 |
| MOTION (sensor) | D15 | P1.13 |
| NCS (sensor) | D18 | P1.15 |
| Extra button | D19 | P0.02 |
| Scroll wheel button | D20 | P0.29 |
| Right button | D21 | P0.31 |

## Event-Driven Architecture

Modules communicate exclusively via the Application Event Manager — no direct module-to-module calls:

- `hw_interface/` modules generate events from hardware (motion_event, wheel_event, battery_event)
- CAF buttons module generates button_event from GPIO scans
- `modules/hid_state.c` collects all input events and assembles HID reports
- BLE/USB transport modules send reports to the host

Reference: `nrf52840gmouse_nrf52840/` configuration is the most complete mouse example in nRF Desktop — use it when implementing features like RGB LEDs, peer selection, or battery charging.

## Debugging

Enable in `prj.conf` as needed:
```
CONFIG_LOG=y
CONFIG_DESKTOP_MOTION_LOG_LEVEL_DBG=y   # Sensor issues
CONFIG_CAF_BUTTONS_LOG_LEVEL_DBG=y      # Button issues
CONFIG_SPI_LOG_LEVEL_DBG=y             # SPI/sensor comms
CONFIG_DESKTOP_BLE_LOG_LEVEL_DBG=y     # BLE pairing issues
```

View logs via RTT (requires J-Link debugger) or USB CDC ACM serial.
