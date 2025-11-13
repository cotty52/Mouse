# nice!nano Mouse Testing Checklist

**Date Started**: November 7, 2025  
**Current Status**: USB HID enumeration working ✅

---

## Phase 1: Basic Functionality ⏳

### USB Connection
- [x] Board recognized in Device Manager as HID mouse
- [ ] Mouse cursor movement detected
- [ ] USB reconnection works after power cycle
- [ ] USB cable quality verified (data transfer)

### LED Indicators
- [ ] Blue LED behavior documented (default behavior?)
- [ ] PWM LED channels working (configured on GPIO 26, 27, 28)
- [ ] LED patterns respond to system state
- [ ] Can manually trigger LED states via code

### Button Input
- [ ] Button 0 (GPIO 0x0007) responds to press
- [ ] Button debouncing working properly
- [ ] Multiple buttons can be read simultaneously
- [ ] Button events generate HID reports

---

## Phase 2: HID Functionality Testing ⏳

### Mouse Reports
- [ ] Left click works
- [ ] Right click works
- [ ] Middle click works
- [ ] Mouse movement (X-axis) works
- [ ] Mouse movement (Y-axis) works
- [ ] Scroll wheel (Z-axis) works

### HID Descriptors
- [ ] Verify HID descriptor matches configuration
- [ ] Boot protocol mouse works (BIOS/legacy mode)
- [ ] Report rate acceptable (check for lag)

### Connection Stability
- [ ] No disconnections during normal use
- [ ] Proper reconnection after sleep
- [ ] USB suspend/resume works

---

## Phase 3: Bluetooth Testing ⏳

### BLE Pairing
- [ ] Device advertises correctly
- [ ] Device name appears in Bluetooth settings
- [ ] Pairing process completes
- [ ] Bond information stored correctly
- [ ] Re-pairing after power cycle works

### BLE HID
- [ ] Mouse works over Bluetooth
- [ ] All buttons work via BLE
- [ ] Movement works via BLE
- [ ] Scroll wheel works via BLE
- [ ] Latency acceptable (<10ms)

### Multi-peer Support
- [ ] Can pair with multiple devices
- [ ] Peer switching works (button 0x0007)
- [ ] Peer erase function works
- [ ] Dongle peer feature works

### BLE Stability
- [ ] Connection stable at 1 meter
- [ ] Connection stable at 3 meters
- [ ] No stuttering during movement
- [ ] Reconnection after range works

---

## Phase 4: Power Management ⏳

### Current Consumption
- [ ] Measure active current (USB)
- [ ] Measure active current (BLE)
- [ ] Measure idle current
- [ ] Measure sleep current
- [ ] Document power states

### Sleep/Wake
- [ ] Device enters sleep after timeout
- [ ] Button press wakes from sleep
- [ ] Movement wakes from sleep
- [ ] Wake latency acceptable
- [ ] No motion events lost on wake

### Battery (Future - pin conflicts need resolution)
- [ ] Battery voltage reading accurate
- [ ] SOC calculation reasonable
- [ ] Low battery warning works
- [ ] Charging detection works
- [ ] Charge status LED works

---

## Phase 5: Configuration & Advanced Features ⏳

### LED Patterns
- [ ] Document default LED patterns
- [ ] Custom patterns for pairing mode
- [ ] Custom patterns for low battery
- [ ] Custom patterns for charging
- [ ] LED stream enable/disable works

### Config Channel
- [ ] USB config channel accessible
- [ ] Settings can be read
- [ ] Settings can be written
- [ ] Settings persist after reboot
- [ ] Factory reset works

### Peer Control
- [ ] Peer select button works
- [ ] Peer erase button combo works
- [ ] Peer status visible (LED?)
- [ ] Dongle peer selector works

---

## Phase 6: Hardware Integration ⏳

### Motion Sensor (PMW3360)
- [ ] SPI communication established
- [ ] Sensor initialization succeeds
- [ ] Motion data valid
- [ ] CPI setting works (test 800/1600/3200)
- [ ] Motion to HID report pipeline works
- [ ] Performance acceptable

### Scroll Wheel
- [ ] QDEC reads encoder correctly
- [ ] Wheel direction correct
- [ ] Detent steps counted accurately
- [ ] Scroll speed appropriate
- [ ] No missed steps during fast scroll

### Physical Buttons
- [ ] Left button mapped correctly
- [ ] Right button mapped correctly
- [ ] Middle button mapped correctly
- [ ] Side buttons (if any) work
- [ ] Button matrix scanning optimal

### Battery System
- [ ] Resolve GPIO pin conflicts
- [ ] Battery ADC reading works
- [ ] Voltage divider calibrated
- [ ] Enable pin works
- [ ] Min/max levels validated

---

## Phase 7: Performance Tuning ⏳

### Latency
- [ ] Measure click-to-action latency
- [ ] Measure motion-to-cursor latency
- [ ] Test LLPM (Low Latency Packet Mode)
- [ ] Compare USB vs BLE latency
- [ ] Optimize if needed

### Reliability
- [ ] 24-hour stress test (USB)
- [ ] 24-hour stress test (BLE)
- [ ] Multi-device switching test
- [ ] Rapid movement test
- [ ] Button spam test

### Power Optimization
- [ ] Optimize sleep timeout
- [ ] Optimize advertising interval
- [ ] Optimize connection interval
- [ ] Test battery life estimate
- [ ] Document power profile

---

## Phase 8: User Experience ⏳

### Firmware Updates
- [ ] Document update procedure
- [ ] Test UF2 update process
- [ ] Verify settings preserved
- [ ] Test recovery from bad flash

### Documentation
- [x] Configuration guide complete
- [ ] User manual created
- [ ] Troubleshooting guide updated
- [ ] Hardware assembly guide
- [ ] BOM (Bill of Materials)

### Quality of Life
- [ ] LED feedback appropriate
- [ ] Button feel/responsiveness good
- [ ] Scroll wheel feel good
- [ ] Size/ergonomics comfortable
- [ ] Weight balanced

---

## Issues Found

### Known Pin Conflicts
1. **Battery charger/measurement** conflicts with UART pins (disabled in config)
2. **Motion buttons** conflict with QDEC pins (disabled in config)
3. **Need to map** final GPIO assignments before PCB design

### Known Limitations
1. **No optical sensor** on nice!nano (need custom hardware)
2. **No physical buttons** yet (need custom hardware)
3. **No battery circuit** yet (need custom hardware)

---

## Immediate Next Steps (Recommended Order)

1. **Test basic mouse movement**
   - Temporarily enable motion buttons to test HID reports
   - Verify cursor moves on screen
   - Confirm USB HID pipeline working end-to-end

2. **Test Bluetooth pairing**
   - Put device in pairing mode
   - Pair with Windows PC
   - Verify BLE HID profile works
   - Test disconnection/reconnection

3. **Design GPIO pin mapping**
   - Resolve all pin conflicts
   - Document final pin assignments
   - Create pin allocation spreadsheet
   - Update DTS overlay with final mappings

4. **Test LED indicators**
   - Verify PWM LEDs work
   - Create test patterns
   - Document LED states
   - Verify pins don't conflict

5. **Hardware design planning**
   - Choose motion sensor (PMW3360 confirmed?)
   - Choose scroll encoder
   - Choose button switches
   - Plan battery/charging circuit
   - Create schematic

---

## Testing Tools Needed

- [ ] USB protocol analyzer (optional - can use Windows USB logs)
- [ ] Bluetooth sniffer (optional - for debugging)
- [ ] Multimeter (for power measurements)
- [ ] Oscilloscope (optional - for SPI debugging)
- [ ] Logic analyzer (optional - for protocol debugging)

---

## Notes & Observations

### Build System
- `--pristine` flag required after partition layout changes
- jsonschema Python package required for board detection
- UF2 output automatic with `CONFIG_BUILD_OUTPUT_UF2=y`

### Flashing Process
- Double-tap reset = bootloader mode (blue LED pulsing)
- Drag-and-drop UF2 = automatic flash and reboot
- Red flashing = bootloader rejecting firmware (address mismatch)

### Development Environment
- VSCode with nRF Connect extension
- West build system
- Git Bash on Windows
- nRF Connect SDK v3.1.1

---

**Keep this checklist updated as you test features!**
