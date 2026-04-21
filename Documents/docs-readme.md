# Documents sub-directory structure

## File Structure
```
Documents
├── docs-readme.md
├── nrf-documents
│   ├── nrf52840-data-sheet.pdf
│   ├── nRF52840-QFAA Reference Layout 1_0
│   ├── nrf52840-qfaa-reference-layout-1_0.zip
│   ├── nrf_connect_sdk_-_latest_board_porting_guide_2026-02-05-12-19-40.pdf
│   └── nrf_connect_sdk_-_latest_zephyr_project_documentation_2026-02-05-12-22-21.pdf
├── zmk-driver-pmw3389-main
├── pmw3389_zephyr_driver
├── nrf-Dongle.3mf
└── update-nice_nano_bootloader-0.9.2_nosd.uf2
```

### Bootloader
- `update-nice_nano_bootloader-0.9.2_nosd.uf2`: The UF2 file for updating the Nice!Nano bootloader to version 0.9.2

This bootloader can be downloaded from the adafruit/Adafruit_nRF52_Bootloader github repository: https://github.com/adafruit/Adafruit_nRF52_Bootloader/releases

To use the bootloader:
  1. Plug the nice!nano into your computer.  
  2. Touch ground and reset together twice within ~0.5 seconds to enter bootloader mode.  
  3. The nice!nano should appear as a USB drive.  
  4. Drag and drop the `update-nice_nano_bootloader-0.9.2_nosd.uf2` file onto the nice!nano USB drive to update the bootloader.  

This process is the same as any other .uf2 file you would use to flash firmware.

### nRF Documents
- `nrf-documents`: A collection of documents related to the nRF52840 SoC, including datasheets, reference layouts, and documentation for the nRF Connect SDK and Zephyr project.

Here is a list of links to helpful resources on the Nordic Semiconductor website:
- [nRF Sample Projects] https://docs.nordicsemi.com/bundle/ncs-3.1.1/page/nrf/samples.html
- [nRF52840 DK] https://docs.nordicsemi.com/bundle/ug_nrf52840_dk/page/UG/dk/ext_programming_support_P20.html
- [nRF52840 Reference Layout] https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/hardware/porting/board_porting.html
- [nRF52840 PMW3360 Driver] https://docs.nordicsemi.com/bundle/ncs-3.1.1/page/nrf/drivers/pmw3360.html
- [nRF52840 Features] https://docs.nordicsemi.com/bundle/ncs-3.1.1/page/nrf/app_dev/device_guides/nrf52/features.html
- [nRF52840 Bootloader Quick Start] https://docs.nordicsemi.com/bundle/ncs-3.1.1/page/nrf/app_dev/bootloaders_dfu/mcuboot_nsib/bootloader_quick_start.html
- [nRF52840 Desktop Application] https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/applications/nrf_desktop/README.html
- [nRF Connect VS Code] https://docs.nordicsemi.com/bundle/nrf-connect-vscode/page/guides/bd_boards_devices.html
- [nRF52840 Board Porting] https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/hardware/porting/board_porting.html#transition_to_the_current_hardware_model
- [nRF52840 Installation] https://docs.nordicsemi.com/bundle/ncs-3.2.2/page/nrf/installation/install_ncs.html
- [nRF52840 Dongle Documentation] https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/boards/nordic/nrf52840dongle/doc/index.html
- [Nordic Academy] https://academy.nordicsemi.com/

### PMW3389 Driver
- `zmk-driver-pmw3389-main`: This is a cloned repository of the PMW3389 driver for ZMK, which can be found at https://github.com/grassfedreeve/zmk-driver-pmw3389. I based my driver off of this repository.
- `pmw3389_zephyr_driver`: Another repository I found after I already finished my driver. As of 2026-04-21 I have not looked through the code, but it has been worked on within the past week so it may provide additional features or optimizations. Here is the link: https://github.com/teamspatzenhirn/pmw3389_zephyr_driver/tree/main. Acording to their bio, this actually was created by Team Spatzenhirn - Uni Ulm, Carolo-Cup team from Ulm University.

The nRF Desktop sample provides a driver for the pmw3360, which is similar. The link to the driver is above; the files can also be found in the ~/ncs directory when installing the nRF Connect SDK.

### nRF Dongle 3MF
- `nrf-Dongle.3mf`: A 3D printable case for the nRF52840 Dongle. Created by AIR VIT-AP and put on MakerWorld: https://makerworld.com/en/models/1107675-opensk-case-for-nrf52480dongle?from=search#profileId-1483028

