# Software sub-directory structure

## File Structure
```
Software
├── boards
├── GEMINI.md
├── notes.md
├── nrf_desktop_x
│   ├── configuration
│   │   ├── common
│   │   ├── nice_nano_v2_nrf52840
│   │   ├── nrf52840dk_nrf52840
│   │   ├── nrf52840dongle_nrf52840
│   │   └── nrf52840gmouse_nrf52840
│   └── ...
├── reference
│   └── zmk_nice_nano
│       ├── arduino_pro_micro_pins.dtsi
│       ├── board.cmake
│       ├── Kconfig
│       ├── Kconfig.board
│       ├── Kconfig.defconfig
│       ├── nice_nano_defconfig
│       ├── nice_nano.dts
│       ├── nice_nano.dtsi
│       ├── nice_nano-pinctrl.dtsi
│       ├── nice_nano_v2.dts
│       ├── nice_nano_v2.yaml
│       ├── nice_nano.yaml
│       └── nice_nano.zmk.yml
├── README.md
└── TODO.md
```

### Overall Structure
The nRF SDK uses Zephyr RTOS as its underlying operating system, so the software structure is based on the Zephyr project structure. I have not done much research on Zephyr, but from a high level overview, a developer must map their specific hardware to the Zephyr hardware model, which involves creating a board definition for the custom mouse. This includes defining the pinout, peripherals, and other features of the hardware that will be used. Additionally, files must be created within the application to map the board definition to the software functions, like which pin on the board is left click.

- `boards` contains the hardware definition for custom boards or microcontrollers; I am only using one, a nice!nano I bought from Aliexpress. These files are almost a direct copy from the ZMK firmware github which can be found in the `reference` directory. 
- `nrf_desktop_x` directory contains the source code for the nRF Desktop sample project, which provides a good starting point for developing a mouse. 
- Within the project directory is `configuration` which is where the hardware is ported to software. These modules are unique to each board, so one must be created for custom hardware.
- `reference` contains the ZMK firmware for the nice!nano. This is used as a reference for the software development of the custom mouse, as both the nRF SDK and ZMK use Zephyr as their underlying RTOS. More information about ZMK can be found on their website: https://zmk.dev/ or their GitHub repository: https://github.com/zmkfirmware/zmk.
- The `GEMINI.md` file is used for AI onboarding, while `notes.md` contains condensed notes written by AI to document the software development process. `TODO.md` is a list for me and an AI to keep track of progress and outline next steps. These files are rudementary, and the information outlined in them should not be taken as final or fully accurate.
