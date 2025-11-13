# nice!nano nRF Desktop Mouse Configuration Guide

## ✅ Working Configuration (November 2025)

This guide documents the **successful** configuration for running nRF Desktop HID Mouse firmware on the nice!nano v2 board with the Adafruit UF2 bootloader.

---

## Hardware Setup

- **Board**: nice!nano v2 (nRF52840-based)
- **Bootloader**: Adafruit nRF52840 Bootloader v0.9.2 **nosd** (no SoftDevice)
- **Flashing Method**: UF2 drag-and-drop via USB mass storage

---

## Critical Configuration Details

### 1. Bootloader Compatibility

**IMPORTANT**: The nice!nano uses the Adafruit UF2 bootloader, which comes in two variants:

- ✅ **nosd (no SoftDevice)**: App starts at `0x1000` (4KB offset)
- ❌ **S140 SoftDevice**: App starts at `0x26000` (152KB offset)

**You MUST match your firmware's partition layout to your bootloader variant.**

### 2. Partition Layout (nosd bootloader)

File: `boards/aliexpress/nice_nano_v2/nice_nano.dtsi`

```dts
&flash0 {
    partitions {
        compatible = "fixed-partitions";
        #address-cells = <1>;
        #size-cells = <1>;

        /* Reserved for MBR only (Adafruit nosd bootloader) */
        mbr_partition: partition@0 {
            reg = <0x00000000 0x00001000>;
        };
        
        /* Application starts at 0x1000 (4KB offset) */
        code_partition: partition@1000 {
            reg = <0x00001000 0x000eb000>;
        };

        /* Storage for settings/configuration */
        storage_partition: partition@ec000 {
            reg = <0x000ec000 0x00008000>;
        };

        /* Bootloader settings at end of flash */
        boot_partition: partition@f4000 {
            reg = <0x000f4000 0x0000c000>;
        };
    };
};
```

### 3. Board Definition Files

**Location**: `boards/aliexpress/nice_nano_v2/`

Key files:
- `board.yml` - Board metadata (vendor: aliexpress, soc: nrf52840)
- `nice_nano_v2_nrf52840.dts` - Main device tree
- `nice_nano_v2_nrf52840_defconfig` - Board default configuration
- `board.cmake` - Build configuration (includes UF2 support)
- `nice_nano-pinctrl.dtsi` - Pin control definitions

**Board defconfig must include**:
```properties
CONFIG_USE_DT_CODE_PARTITION=y
CONFIG_BUILD_OUTPUT_UF2=y
CONFIG_MPU_ALLOW_FLASH_WRITE=y
```

### 4. Application Configuration

**Location**: `nrf_desktop_3/configuration/nice_nano_v2_nrf52840/`

Key features enabled:
- HID Peripheral (Mouse)
- USB HID support
- Bluetooth LE
- PWM LEDs for status indication
- QDEC for scroll wheel simulation
- Button support (GPIO)

**Do NOT enable** (without hardware modifications):
- PMW3360 optical sensor (not present on nice!nano)
- Battery charger/measurement on default UART pins (conflicts)

---

## Build Process

### Prerequisites

1. **nRF Connect SDK** v3.1.1+ installed
2. **Python 3.10+** with required packages:
   ```bash
   pip install jsonschema
   ```
3. **West** meta-tool installed
4. **BOARD_ROOT** set in `CMakeLists.txt`:
   ```cmake
   set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
   ```

### Build Commands

```bash
# Navigate to application directory
cd nrf_desktop_3

# Clean build with new configuration
west build -b nice_nano_v2/nrf52840 --pristine

# Build output: build/zephyr/zephyr.uf2
```

### Verify UF2 Configuration

```bash
python ../reference/uf2conv.py --info build/zephyr/zephyr.uf2
```

Expected output:
```
--- UF2 File Header Info ---
Family ID is NRF52840, hex value is 0xada52840
Target Address is 0x00001000
All block flag values consistent, 0x2000
```

**Key check**: `Target Address is 0x00001000` confirms nosd bootloader compatibility.

---

## Flashing Procedure

1. **Enter bootloader mode**:
   - Double-tap the reset button on nice!nano
   - Board appears as USB mass storage device "NICENANO"
   - Blue LED should be pulsing

2. **Flash firmware**:
   - Drag and drop `build/zephyr/zephyr.uf2` onto the NICENANO drive
   - Board will automatically reboot after flashing

3. **Verify success**:
   - Board should appear in Device Manager as "HID-compliant mouse"
   - LEDs should indicate normal operation (not flashing red)
   - Mouse should be functional

---

## Troubleshooting

### Red LED Continuously Flashing After Upload

**Problem**: Bootloader/firmware partition mismatch

**Solutions**:

1. **Check UF2 target address**:
   ```bash
   python reference/uf2conv.py --info build/zephyr/zephyr.uf2
   ```
   - Should show `0x00001000` for nosd bootloader
   - Shows `0x00026000` for S140 bootloader

2. **Wrong bootloader installed**:
   - If you have S140 bootloader but firmware built for nosd, flash the nosd bootloader
   - Or rebuild firmware with S140 partition layout

3. **Rebuild with pristine flag**:
   ```bash
   west build -b nice_nano_v2/nrf52840 --pristine
   ```

### Build Errors

**Error**: `No module named 'jsonschema'`
```bash
pip install jsonschema
# or
"C:/Program Files/Python313/python.exe" -m pip install jsonschema
```

**Error**: `Error finding board: nice_nano_v2`
- Verify `BOARD_ROOT` is set in `CMakeLists.txt`
- Check board files exist in `boards/aliexpress/nice_nano_v2/`
- Use correct board identifier: `nice_nano_v2/nrf52840`

### Device Not Recognized

**Check**:
1. USB cable supports data (not charge-only)
2. Board enters bootloader mode (blue LED pulsing)
3. Windows recognizes USB device in Device Manager

---

## Testing Features

### Working Features (as of Nov 2025)

✅ **USB HID Mouse**: Device recognized as HID mouse in Windows
✅ **Bluetooth**: SoftDevice Controller (SDC) stack enabled
✅ **PWM LEDs**: Status indication on GPIO pins
✅ **QDEC**: Scroll wheel simulation using encoder interface
✅ **Buttons**: GPIO button matrix support
✅ **USB**: Full USB HID support

### Not Yet Tested

⚠️ **Motion sensing**: No optical sensor hardware
⚠️ **Battery monitoring**: Pin conflicts need resolution
⚠️ **Wireless pairing**: Not yet verified
⚠️ **Sleep/wake**: Power management testing needed

---

## Next Steps

### Phase 1: Core Functionality Testing
1. ✅ Verify USB HID mouse enumeration
2. ⏳ Test button inputs (GPIO)
3. ⏳ Test LED status indicators
4. ⏳ Test QDEC wheel simulation

### Phase 2: Wireless Features
1. ⏳ Test Bluetooth LE pairing
2. ⏳ Test BLE HID over GATT
3. ⏳ Test peer control/switching
4. ⏳ Verify wireless mouse functionality

### Phase 3: Hardware Integration
1. ⏳ Add motion sensor (PMW3360) on SPI
2. ⏳ Add physical scroll wheel encoder
3. ⏳ Add mouse buttons (left/right/middle)
4. ⏳ Resolve battery monitoring pin conflicts
5. ⏳ Add battery charging circuit

### Phase 4: Optimization
1. ⏳ Power consumption optimization
2. ⏳ Sleep/wake timing tuning
3. ⏳ Motion sensor CPI configuration
4. ⏳ Debounce and responsiveness tuning

### Phase 5: Custom Features
1. ⏳ Custom LED patterns
2. ⏳ DPI switching via button
3. ⏳ Profile switching
4. ⏳ Configuration via USB/BLE

---

## Key Lessons Learned

### 1. Bootloader Compatibility is Critical
- **Always** verify your UF2 target address matches your bootloader
- Red flashing LED = bootloader rejecting firmware due to address mismatch
- nosd bootloader: app @ 0x1000, S140 bootloader: app @ 0x26000

### 2. Partition Layout Must Match Bootloader
- DTS partition definitions control where linker places code
- `code_partition` starting address must match bootloader expectations
- Clean builds (`--pristine`) required after partition changes

### 3. SoftDevice vs SoftDevice Controller
- Modern nRF Connect SDK uses **SoftDevice Controller (SDC)** - pure software stack
- SDC doesn't require SoftDevice binary blob in flash
- But partition layout can still reserve space for compatibility

### 4. UF2 Format Simplifies Flashing
- No need for J-Link programmer
- Drag-and-drop flashing via USB mass storage
- `CONFIG_BUILD_OUTPUT_UF2=y` enables automatic UF2 generation

### 5. Pin Conflicts Need Resolution
- nice!nano has limited pins
- Battery charger/measurement pins conflict with UART debug
- Motion sensor buttons conflict with QDEC
- Plan pin usage carefully before hardware design

---

## Reference Files

### Successful Build Configuration (nrf_desktop_3)
- **Board root**: `boards/aliexpress/nice_nano_v2/`
- **Application config**: `nrf_desktop_3/configuration/nice_nano_v2_nrf52840/`
- **Build output**: `nrf_desktop_3/build/zephyr/zephyr.uf2`

### Key Configuration Values
```
BOARD=nice_nano_v2/nrf52840
SOC=nrf52840
FLASH_LOAD_OFFSET=0x1000
ROM_START_OFFSET=0
BUILD_OUTPUT_UF2=y
```

### Useful Commands
```bash
# Check UF2 info
python reference/uf2conv.py --info <file.uf2>

# Verify build configuration
grep CONFIG_FLASH_LOAD_OFFSET build/zephyr/.config

# List board configurations
west boards | grep nice_nano

# Clean rebuild
west build -b nice_nano_v2/nrf52840 --pristine
```

---

## Additional Resources

- **Adafruit Bootloader**: https://github.com/adafruit/Adafruit_nRF52_Bootloader
- **nRF Connect SDK**: https://developer.nordicsemi.com/nRF_Connect_SDK/
- **nRF Desktop Documentation**: See `*.rst` files in `nrf_desktop_3/`
- **UF2 Format**: https://github.com/microsoft/uf2

---

**Document Version**: 1.0  
**Last Updated**: November 7, 2025  
**Status**: ✅ Working - USB HID Mouse Enumeration Confirmed
