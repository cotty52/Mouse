# nRF Desktop Mouse on nice!nano v2

## Status: ✅ WORKING (as of November 7, 2025)

This is a testing environment for a custom wireless mouse project using the **nRF52840 nice!nano v2** microcontroller running Nordic's **nRF Desktop** sample application.

### Quick Summary

The nice!nano board uses an **Adafruit UF2 bootloader** (nosd variant), which requires specific partition layout configuration to work with nRF Connect SDK. After proper configuration, the board successfully enumerates as an **HID-compliant mouse** in Windows.

## 📖 Documentation

**See [CONFIGURATION_GUIDE.md](CONFIGURATION_GUIDE.md) for complete setup instructions, troubleshooting, and configuration details.**

## Project Structure

```
boards/
  aliexpress/
    nice_nano_v2/          ← Custom board definition for Zephyr
nrf_desktop_1/             ← Standard nRF52840 DK configuration (reference)
nrf_desktop_2/             ← Experimental nested config (archived)
nrf_desktop_3/             ← ✅ WORKING nice!nano configuration
reference/
  uf2conv.py               ← UF2 utility for validation
  update-nice_nano_*       ← Bootloader files
```

## Quick Start

### Build
```bash
cd nrf_desktop_3
west build -b nice_nano_v2/nrf52840 --pristine
```

### Flash
1. Double-tap reset on nice!nano (enters bootloader mode)
2. Drag and drop `build/zephyr/zephyr.uf2` to NICENANO drive
3. Board reboots and appears as HID mouse

### Verify
```bash
python reference/uf2conv.py --info build/zephyr/zephyr.uf2
# Should show: Target Address is 0x00001000
```

## Key Technical Details

- **Bootloader**: Adafruit nRF52840 v0.9.2 nosd (app @ 0x1000)
- **BLE Stack**: SoftDevice Controller (SDC) - no SoftDevice blob needed
- **Flash Layout**: Code @ 0x1000-0xEC000, Storage @ 0xEC000-0xF4000
- **Build Output**: UF2 format for drag-and-drop flashing

## What Works

✅ USB HID mouse enumeration  
✅ Board boots without errors  
✅ UF2 bootloader compatibility  
✅ Proper partition layout  

## Next Steps

See **CONFIGURATION_GUIDE.md** → Next Steps section for:
- Button input testing
- Bluetooth LE pairing
- Motion sensor integration
- Hardware design finalization

## Important Notes

⚠️ **Bootloader compatibility is critical**: Your UF2 target address (0x1000 or 0x26000) must match your bootloader variant (nosd or S140).

⚠️ **Pin conflicts exist**: Battery monitoring and some features have GPIO conflicts that need resolution before full hardware integration.

## Resources

- Configuration Guide: [CONFIGURATION_GUIDE.md](CONFIGURATION_GUIDE.md)
- AI Instructions: [.github/copilot-instructions.md](.github/copilot-instructions.md)
- nRF Desktop Docs: See `*.rst` files in `nrf_desktop_3/`