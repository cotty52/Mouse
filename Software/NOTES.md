**Build Command**
I build using the nrf connect vscode extension. Using the command line to build is not necessary, but if you think it would be helpful, this is the command you should use. It is an example, so change the file path for `build_*` if I move on to a new build configuration.
west build --build-dir /home/christian/ncs_projects/nrf_desktop-2026-2-5/build_custom_2 /home/christian/ncs_projects/nrf_desktop-2026-2-5

**nice!nano board features**:
- 1MB of Flash and 256KB of RAM
- 32.768 kHz oscillator on board for real-time clock capabilities

**From Adafruit Bootloader GitHub**: https://github.com/adafruit/Adafruit_nRF52_Bootloader
Making your own UF2

To create your own UF2 DFU update image, simply use the Python conversion script on a .bin file or .hex file, specifying the family as 0xADA52840 (nRF52840) or 0x621E937A (nRF52833).
```
nRF52840
uf2conv.py firmware.hex -c -f 0xADA52840
```
```
nRF52833
uf2conv.py firmware.hex -c -f 0x621E937A
```

If using a .bin file with the conversion script you must specify application address with the -b switch, this address depend on the SoftDevice size/version e.g S140 v6 is 0x26000, v7 is 0x27000
```
nRF52840
uf2conv.py firmware.bin -c -b 0x26000 -f 0xADA52840
```
```
nRF52833
uf2conv.py firmware.bin -c -b 0x27000 -f 0x621E937A
```

To create a UF2 image for bootloader from a .hex file using separated family of 0xd663823c
```
uf2conv.py bootloader.hex -c -f 0xd663823c
```

**PCB Pinout**
The sensor I have is pmw3389. The gmouse example uses pmw3360 files which may or may not be similar enough to use for testing.
Left button: D0, 0.08
Thumb 1: D2, 0.17
Thumb 2: D3, 0.20
Encoder A: D5, 0.22
Encoder B: D6, 1.00
2.4 GHz (switch): D7, 0.11
Bluetooth (switch): D8, 1.04
NRESET (sensor): D9, 1.06
MISO (sensor): D10, 0.09
MOSI (sensor): D16, 0.10
SCLK (sensor): D14, 1.11
MOTION (sensor): D15, 1.13
NCS (sensor): D18, 1.15
Extra button: D19, 0.02
Scroll wheel button: D20, 0.29
Right button: D21, 0.31

PMW3389        DK Arduino Pin    nRF52840 GPIO
────────       ──────────────    ─────────────
SCK            D13               P1.15
MOSI           D11               P1.13
MISO           D12               P1.14
NCS (CS)       D10               P1.12
MOTION (IRQ)   D9                P1.11
NRESET         D8                P1.10
VDD            3V3
GND            GND