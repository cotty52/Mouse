# AI Coding Instructions - nRF Desktop Mouse Project

## Project Context

This is a **custom wireless HID mouse** built on Nordic's **nRF Desktop** reference application running on Zephyr RTOS. The target hardware is the **nice!nano v2** (nRF52840) with an Adafruit UF2 bootloader and PMW3389 optical sensor.

**Critical**: This is NOT a standard Zephyr application - it's a Nordic nRF Connect SDK sample with custom board definitions and a two-layer configuration architecture.

## Architecture Overview

### Event-Driven Modular Design

The application uses **Common Application Framework (CAF)** with `app_event_manager` as the central event bus. All modules communicate via typed events:

- **Event definitions**: `nrf_desktop_5/src/events/*.h` (e.g., `motion_event.h`, `hid_event.h`, `usb_event.h`)
- **Module pattern**: Each module subscribes to events using `APP_EVENT_LISTENER()` and `APP_EVENT_SUBSCRIBE()` macros
- **Example**: `motion_sensor.c` submits `motion_event`, `hid_state.c` receives it and generates HID reports

```c
// Typical module pattern
APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, motion_event);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
```

Key modules in `src/modules/`: `hid_state.c`, `usb_state.c`, `ble_*.c`, `dfu.c`  
Hardware interfaces in `src/hw_interface/`: `motion_sensor.c`, `buttons_sim.c`, `battery_meas.c`

## Two-Layer Configuration System

### Layer 1: Board Definition (`boards/aliexpress/nice_nano_v2/`)

Defines the **hardware** to Zephyr - reusable across projects:

- **`board.yml`**: Board metadata (vendor: aliexpress, SoC: nrf52840)
- **`nice_nano_v2_nrf52840.dts`**: Complete hardware blueprint - GPIO, SPI, I2C, UART peripherals
- **`nice_nano_v2_nrf52840_defconfig`**: Default Kconfig (must include `CONFIG_BUILD_OUTPUT_UF2=y` for UF2 bootloader)
- **`arduino_pro_micro_pins.dtsi`**: Pin name aliases (D0-D21, A0-A8) for readable overlay files

### Layer 2: Application Configuration (`nrf_desktop_5/configuration/nice_nano_v2_nrf52840/`)

Configures **nRF Desktop features** for this specific board:

- **`prj.conf`**: Feature switches (USB, BLE, mouse/keyboard mode, enabled modules)
- **`app.overlay`**: Maps software to hardware pins (button GPIOs, SPI sensors, LED assignments)
- **`pm_static.yml`**: **CRITICAL** - Flash memory partitions for UF2 bootloader compatibility
- **`*_def.h` headers**: Define mappings (buttons→key IDs, key IDs→HID usages)

**Example**: To enable PMW3389 sensor:
1. In `prj.conf`: `CONFIG_PMW3389=y`, `CONFIG_SPI=y`
2. In `app.overlay`: Define SPI1 bus with CS/IRQ pins
3. In `hid_keymap_def.h`: Map button IDs to mouse clicks

## Critical Build Requirements

### UF2 Bootloader Memory Layout

The nice!nano uses Adafruit's UF2 bootloader (no SoftDevice variant). **Application MUST start at 0x1000**:

```yaml
# pm_static.yml - DO NOT MODIFY ADDRESSES
mcuboot_primary:
  address: 0x1000  # Bootloader expects app here
  size: 0xF7000
settings_storage:
  address: 0xF8000
  size: 0x8000
```

Verify built UF2: `python reference/uf2conv.py --info build/zephyr/zephyr.uf2`

### CMake Configuration Quirk

**DO NOT use `APPLICATION_CONFIG_DIR` with `zephyr_include_directories()`** - causes Kconfig path duplication. Instead:

```cmake
# nrf_desktop_5/CMakeLists.txt
include_directories(
  "configuration/common"
  "${CMAKE_CURRENT_LIST_DIR}/configuration/nice_nano_v2_nrf52840"
)
```

## Essential Workflows

### Building for nice!nano v2

```bash
cd nrf_desktop_5
west build -b nice_nano_v2/nrf52840 --pristine
```

**Custom board discovery**: The `BOARD_ROOT` variable in `CMakeLists.txt` points to `../` to find `boards/aliexpress/nice_nano_v2/`.

### Flashing the Device

1. **Enter bootloader**: Double-tap reset button on nice!nano
2. **Flash**: Drag `build/zephyr/zephyr.uf2` to the `NICENANO` USB drive
3. **Verify**: Device enumerates as HID mouse/keyboard via USB or BLE

### External Dependencies

The `west.yml` manifest defines additional drivers:

```yaml
# nrf_desktop_5/west.yml
projects:
  - name: pmw3389_zephyr_driver
    remote: teamspatzenhirn
    path: modules/drivers/pmw3389
```

Run `west update` after modifying `west.yml` to fetch external modules.

## Project-Specific Conventions

### Kconfig Naming Patterns

- `CONFIG_DESKTOP_*`: Application feature flags (e.g., `CONFIG_DESKTOP_BT`, `CONFIG_DESKTOP_USB_ENABLE`)
- `CONFIG_DESKTOP_PERIPHERAL_TYPE_*`: Device role (`MOUSE`, `KEYBOARD`, `DONGLE`)
- `CONFIG_DESKTOP_MOTION_*`: Motion sensor subsystem options
- `CONFIG_CAF_*`: Common Application Framework modules (buttons, LEDs, power management)

### Device Tree Overlays

Use Pro Micro pin aliases in overlays, not raw GPIO numbers:

```dts
// Good - readable
button_left: button_left {
    gpios = <&gpio0 DT_GPIO_PIN(DT_ALIAS(d6), gpios) (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
};

// Also good - direct pin reference with comment
cs-gpios = <&gpio0 9 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;  /* D9 */
```

### Header File Guards

Configuration headers in `configuration/*/` use unique struct enforcement (not `#pragma once`):

```c
// buttons_def.h
const struct {} buttons_def_include_once;  // Prevents multiple includes
```

## Common Modifications

### Switching from Mouse to Keyboard

1. **`prj.conf`**: Change `CONFIG_DESKTOP_PERIPHERAL_TYPE_MOUSE=y` → `CONFIG_DESKTOP_PERIPHERAL_TYPE_KEYBOARD=y`
2. **`prj.conf`**: Add `CONFIG_DESKTOP_HID_KEYMAP_ENABLE=y`
3. **`app.overlay`**: Replace button nodes with `kscan` (keyboard matrix) node
4. Create **`nice_nano_v2_nrf52840.keymap`** defining key layout

### Integrating Motion Sensors

Current config uses **simulated motion** (`motion_buttons.c`). To use PMW3389:

1. **`prj.conf`**: Remove `CONFIG_DESKTOP_MOTION_BUTTONS_ENABLE=y`, add `CONFIG_PMW3389=y`
2. **`app.overlay`**: Enable `&spi1` and define sensor node with CS/IRQ pins
3. **Verify**: Driver source is in `modules/drivers/pmw3389` (from `west.yml`)

### Adding New Modules

1. Create `src/modules/my_module.c` with event subscriptions
2. Add to `src/modules/CMakeLists.txt`: `add_subdirectory_ifdef(CONFIG_MY_MODULE my_module)`
3. Create `src/modules/Kconfig.my_module` with enable flag
4. Include in `src/modules/Kconfig`: `rsource "Kconfig.my_module"`

## Known Issues and Workarounds

- **Kconfig dependency loops**: Remove `if BOARD_*` blocks from `boards/*/Kconfig.defconfig`
- **Obsolete Kconfig symbols**: Check `Kconfig.deprecated` if build warns about undefined symbols
- **BT_ID_MAX too low**: Set `CONFIG_BT_ID_MAX=4` minimum for bonding support
- **Missing *_def.h headers**: Create empty files from `nrf52840dk_nrf52840` templates as placeholders

## Reference Documentation

- **GEMINI.md**: High-level project overview and build instructions
- **notes.md**: Detailed configuration architecture and build fix history
- **TODO.md**: Current development tasks and integration roadmap
- **nrf_desktop_5/README.rst**: Nordic's official nRF Desktop documentation
- **nrf_desktop_5/doc/*.rst**: Per-module documentation with event tables
