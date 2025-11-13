# nRF Desktop Mouse - AI Coding Agent Instructions

## Project Overview

This is a testing environment for adapting Nordic Semiconductor's nRF Desktop sample application to work with the nice!nano v2 board (nRF52840-based). The nice!nano uses an Adafruit UF2 bootloader instead of the Nordic development kit bootloader, requiring specific modifications to the nRF Connect SDK workflow.

**Key Goal**: Make the nRF Desktop HID peripheral (wireless mouse) firmware work with nice!nano hardware using the UF2 flashing format.

## Architecture

### Multiple Test Variants

The workspace contains three parallel nRF Desktop copies (`nrf_desktop_1`, `nrf_desktop_2`, `nrf_desktop_3`) representing different experimental configurations. Each tests different approaches to nice!nano integration:

- **nrf_desktop_1**: Works with standard nRF52840 dev kit (`nrf52840dk_nrf52840`)
- **nrf_desktop_2**: Nested configuration approach - has `configuration/nice_nano_nrf52840/configuration/nrf52840gmouse_nrf52840/`
- **nrf_desktop_3**: Direct nice!nano v2 support - has `configuration/nice_nano_v2_nrf52840/configuration/nice_nano_v2_nrf52840/`

### Event-Driven Modular Design

The nRF Desktop application uses the Common Application Framework (CAF) with an event-driven architecture:

- **Application Event Manager**: Central event bus connecting all modules
- **Modules** (`src/modules/`): Independent components (BLE, HID, motion, battery, etc.) that communicate via events
- **Hardware Interfaces** (`src/hw_interface/`): Abstractions for buttons, LEDs, sensors
- **Events** (`src/events/`): Typed event definitions used for module communication

See module documentation in `*.rst` files (especially `description.rst`, `modules.rst`) for event flow diagrams.

## Configuration System

### Per-Board Configuration Pattern

The build system uses `configuration/${NORMALIZED_BOARD_TARGET}/` directories:

```
configuration/
  common/                    # Shared HID descriptors and definitions
  nrf52840dk_nrf52840/      # Dev kit configuration
    prj.conf                 # Main Kconfig settings
    app.overlay              # DTS hardware overlay
    buttons_def.h            # Button matrix definition
    led_state_def.h          # LED configuration
    pm_static*.yml           # Partition manager layouts
    sysbuild.conf            # Sysbuild configuration
    images/                  # Bootloader configs
      mcuboot/
      b0/
```

### Kconfig Configuration Layers

1. **Application Kconfig** (`Kconfig`, `Kconfig.hid`, `Kconfig.ble`): nRF Desktop-specific options
2. **Per-board `prj.conf`**: Enables/disables modules and sets hardware parameters
3. **Multiple build variants**: `prj_release.conf`, `prj_dongle.conf`, `prj_keyboard.conf`, etc.

### Critical: APPLICATION_CONFIG_DIR

The CMake variable `APPLICATION_CONFIG_DIR` is set in both:
- `CMakeLists.txt`: `set(APPLICATION_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/configuration/\${NORMALIZED_BOARD_TARGET}")`
- `sysbuild/CMakeLists.txt`: Similar pattern for bootloader images

## Building with nRF Connect SDK

### Prerequisites

- **nRF Connect SDK**: This project requires the full Nordic toolchain (tested with v3.1.1)
- **Toolchain path**: `C:/ncs/v3.1.1/` (Windows) or adjust for WSL
- **West**: Nordic's meta-tool for multi-repo management
- **BOARD_ROOT**: Set in CMakeLists.txt to point to custom `boards/` directory: `set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)`

### Build Commands

```bash
# Standard build (from nRF Connect SDK environment)
west build -b nrf52840dk/nrf52840 nrf_desktop_1

# Build for nice!nano with UF2 output
west build -b nice_nano_v2/nrf52840 nrf_desktop_3 --pristine

# Build with specific configuration variant
west build -b nrf52840dk/nrf52840 nrf_desktop_1 -- -DCONF_FILE=prj_release.conf

# Sysbuild (includes bootloader)
west build -b nrf52840dk/nrf52840 nrf_desktop_1
```

**Output artifacts**:
- Standard: `build/zephyr/zephyr.hex`, `zephyr.bin`
- With UF2: `build/zephyr/zephyr.uf2` (drag-and-drop flashing)

### VSCode Integration

The workspace uses nRF Connect for VS Code extension:
- `.vscode/settings.json` lists applications: `nrf_desktop_1`, `nrf_desktop_2`, `nrf_desktop_3`
- Use extension's build buttons or native West CLI

## nice!nano Board Support

### Custom Board Definition

Location: `boards/aliexpress/nice_nano_v2/`

**Critical**: Set `BOARD_ROOT` in application's `CMakeLists.txt` before `find_package(Zephyr)`:
```cmake
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
```

Key files for Zephyr v2 board system:
- **`board.yml`**: Board metadata (vendor: aliexpress, soc: **nrf52840**) - **Required**
- **`nice_nano_v2_nrf52840.dts`**: Device tree - **must be named `{board}_{soc}.dts`**
- **`nice_nano_v2_nrf52840_defconfig`**: Board defaults - **must be named `{board}_{soc}_defconfig`**
- **`board.cmake`**: Includes `uf2.board.cmake` for UF2 generation
- **`Kconfig.defconfig`**: Board-specific Kconfig defaults (conditional on board selection)
- **`nice_nano-pinctrl.dtsi`**: Pinctrl definitions shared across variants
- **Do NOT include**: `Kconfig.board` (v1 style), multiple YAML files, or manual SOC selection

### UF2 Bootloader Considerations

The nice!nano uses Adafruit's bootloader:
- **Double-tap reset** to enter bootloader mode
- **Flash via USB mass storage**: Copy `.uf2` file to mounted drive
- **No SEGGER J-Link**: Cannot use `west flash` with standard Nordic tools
- **Partition layout**: Must match bootloader expectations (see `reference/nice_nano_bootloader-*.hex`)

## Module Configuration Examples

### Enabling Mouse Hardware Modules

In `prj.conf`:
```properties
CONFIG_DESKTOP_ROLE_HID_PERIPHERAL=y
CONFIG_DESKTOP_PERIPHERAL_TYPE_MOUSE=y

# Motion sensor (PMW3360 optical sensor)
CONFIG_DESKTOP_MOTION_SENSOR_PMW3360_ENABLE=y
CONFIG_DESKTOP_MOTION_SENSOR_CPI=1600

# Wheel (QDEC peripheral)
CONFIG_DESKTOP_WHEEL_ENABLE=y

# Buttons (GPIO matrix)
CONFIG_CAF_BUTTONS=y
CONFIG_CAF_BUTTONS_POLARITY_INVERSED=y

# Battery measurement (ADC)
CONFIG_DESKTOP_BATTERY_MEAS=y
CONFIG_ADC=y

# Bluetooth & USB
CONFIG_DESKTOP_USB_ENABLE=y
CONFIG_DESKTOP_BLE_USE_DEFAULT_ID=y
```

### DTS Overlays for Hardware

Create `app.overlay` to override board defaults:
```dts
/ {
    buttons {
        compatible = "gpio-keys";
        button0: button_0 {
            gpios = <&gpio0 13 GPIO_PULL_UP>;
        };
    };
};
```

## Common Patterns

### Adding New Hardware Configuration

1. **Copy reference design**: Start from `configuration/nrf52840gmouse_nrf52840/`
2. **Modify `prj.conf`**: Enable/disable modules for your hardware
3. **Create DTS overlay**: Define GPIO pins, sensors, buses in `app.overlay`
4. **Update `_def.h` files**: Button matrices, LED mappings in config directory
5. **Test iteratively**: Build with `--pristine` to ensure clean builds

### Debugging Build Issues

- **Check `build/CMakeCache.txt`**: Verify `APPLICATION_CONFIG_DIR` points to correct path
- **Review `build/build_info.yml`**: Confirms board, DTS files, Kconfig files used
- **Kconfig warnings**: Use `west build -t menuconfig` to inspect configuration
- **DTS errors**: Check `build/zephyr/zephyr.dts` for final merged device tree
- **Board config errors**: In Zephyr's v2 board system, `CONFIG_BOARD_*` and `CONFIG_SOC_SERIES_*` symbols are auto-set by `board.yml` - never manually set them in defconfig files
- **Kconfig dependency warnings**: Features requiring hardware (QDEC, PWM LEDs, SPI sensors) need corresponding DTS nodes. Add DTS nodes in app.overlay or board files
- **SOC auto-configuration**: The build system automatically sets SOC-related configs based on `board.yml` - don't add `CONFIG_SOC_*` to defconfig

### Configuration Hierarchy in nrf_desktop_2 & nrf_desktop_3

These variants use nested configuration directories, creating an unusual pattern:
- `configuration/nice_nano_*/configuration/TARGET_BOARD/prj.conf`

This appears to be an experiment in configuration aliasing/inheritance.

## Critical Files Reference

- **`CMakeLists.txt`**: Sets up build, includes `src/` modules
- **`Kconfig`**: Top-level menu and module inclusion
- **`configuration/common/hid_report_desc.c`**: USB/BLE HID descriptors
- **`src/main.c`**: Minimal - just initializes Event Manager
- **`src/modules/`**: All application logic (BLE, HID, motion, etc.)
- **`sysbuild/CMakeLists.txt`**: Multi-image build (app + bootloader)

## Reference Material

- **`reference/zmk_nice_nano/`**: ZMK's nice!nano board definition for comparison
- **Documentation**: All `*.rst` files are nRF Desktop upstream documentation

## Testing Approach

This workspace uses experimentation via parallel copies rather than git branches. When making changes:

1. Work in one `nrf_desktop_*` directory
2. Compare differences with others using file diff tools
3. Document what works in `readme.md`
4. Build artifacts go to `build/` (gitignored)

## nice!nano Hardware Configuration Strategy

When adapting nRF Desktop for nice!nano:

1. **Board configuration requirements**:
   - **`board.yml`**: Must specify SOC name: `nrf52840` (matches soc directory name)
   - **Board defconfig**: `CONFIG_BUILD_OUTPUT_UF2=y` for UF2 bootloader support
   - **Kconfig files**: Board symbol must match board name (e.g., `BOARD_NICE_NANO_V2` not `BOARD_NICE_NANO`)
   - **Do NOT set** `CONFIG_SOC_*` or `CONFIG_BOARD_*` in defconfig - auto-configured by `board.yml`

2. **Add DTS nodes to support application features**:
   - **PWM LEDs**: Add `pwmleds0` and `pwmleds1` nodes in app.overlay, enable `&pwm0` and `&pwm1`
   - **QDEC wheel**: Enable `&qdec` with pinctrl configuration for encoder simulation
   - **Pinctrl definitions**: Add `pwm0_default`, `pwm1_default`, `qdec_default` to board's pinctrl file
   - **ADC**: Already in nice!nano base DTS, just enable in overlay if needed

3. **Motion sensor alternatives**:
   - No PMW3360 optical sensor → Use `CONFIG_DESKTOP_MOTION_BUTTONS_ENABLE=y`
   - Map buttons to simulate motion: `CONFIG_DESKTOP_MOTION_BUTTONS_UP/DOWN/LEFT/RIGHT_KEY_ID`

4. **Key principle**: Add hardware support in DTS rather than disabling features in Kconfig
   - Better for testing complete application architecture
   - Easier to swap in real hardware later
   - Maintains full feature set for validation

---

**When suggesting changes**: Always verify which `nrf_desktop_*` variant is being modified and whether the target board configuration exists before proposing file edits.
