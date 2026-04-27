# Custom Wireless Mouse

This project contains software and hardware designs for a custom wireless mouse. This is an initial prototype intended for me to learn about hardware design and embedded software development. I combined hardware and software files into one repository for easier management. As of 2026-04-21, the design supports basic mouse functionality, including movement, button presses, and scrolling, and is powered by a 3.7V Li-ion battery, rechargeable via USB-C.

## File Structure

```
Mouse/
├── Documents/        # Datasheets, bootloader UF2, 3D models
├── Guide/
│   └── README.md     # New machine setup guide (start here after cloning)
├── KiCad/            # PCB schematics and layout
└── Software/
    ├── PROJECT_GUIDE.md  # Architecture, task tracking, module reference
    └── ...
```

- `Documents` contains datasheets, reference layouts, and documentation for the nRF Connect SDK and Zephyr project.
- `Guide` contains the new machine setup guide (`Guide/README.md`) and dated snapshots of known-good board/config files.
- `KiCad` contains the KiCad project files for the PCB design (schematics, PCB layout, Gerber files).
- `Software` contains firmware source, board definitions, and configuration files. See `Software/PROJECT_GUIDE.md` for details.

*Disclosure* - Some of the content (notes, drivers, etc) was generated with AI assistance.

## Hardware Design

The hardware design is based on the nRF52840 because of its popularity/availability, built in Bluetooth, low power consumption, and integration with the nRF SDK for software development. The PCB was designed using KiCad and currently relies on a nRF52840 nice!nano microcontroller for higher level development in the early stages. The microcontroller includes a USB-C port for charging and firmware flashing, a charging protection circuit, and enough GPIO for this project. The design also includes a PMW3389 optical sensor for mouse movement, which is supported by the nRF SDK and Zephyr.

## Software Design

The software is developed using the nRF Connect SDK, which is based on the Zephyr RTOS. The nRF Connect VS Code extension is used for development, which provides a convenient interface for firmware development. The base structure is copied directly from the nRF Desktop sample project, which provides a good starting point for developing a Bluetooth mouse. The sample provides support for Bluetooth connectivity, mouse movement, button presses, and scrolling out of the box, with additional features available to be toggled on/off using the Kconfig GUI. The sample also provides sample mouse configurations as well as a PMW3360 driver, which is similar to the PMW3389. These files serve as a reference for the development of custom hardware and application files for the custom mouse.

## Future Improvements

The next step is to design the PCB without the nice!nano which will require designing the PCB with the nRF52840 SoC directly. This will involve further researching the nRF52840 for proper implementation as well as designing power management and charging circuits. I will also need to decide on whether to use an SoC with an included antenna or design one myself.

On the software side, the next step is to develop custom application code for the mouse which will allow users to customize the behavior of the mouse, such as button mapping and sensitivity settings.
