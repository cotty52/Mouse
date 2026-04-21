# Custom Wireless Mouse

This project contains hardware and software designs for a custom wireless mouse. This is an initial prototype intended for me to learn about hardware design and embedded software development. I combined hardware and software files into one repository for easier management. As of 2026-04-21, the design supports basic mouse functionality, including movement, button presses, and scrolling, and is powered by a 3.7V Li-ion battery, rechargeable via USB-C.

## File Structure
```
Mouse
├── Documents
│   ├── docs-readme.md
│   └── ...
├── KiCad
├── README.md
├── Software
│   ├── software-readme.md
│   └── ...
└── zmk_nice_nano
```

- ./Documents contains various documents related to the project, including datasheets, reference layouts, and documentation for the nRF Connect SDK and Zephyr project. See `Documents/docs-readme.md` for more details.
- ./KiCad contains the KiCad project files for the PCB design. This includes schematics, PCB layout, and Gerber files.
- ./Software contains the software development files for the project, including source code, documentation, and build files. See `Software/software-readme.md` for more details.
- ./zmk_nice_nano contains the Zephyr port of the zmk firmware for the nice!nano. This is used as a reference for the software development of the custom mouse, as both the nRF SDK and ZMK use Zephyr as their underlying RTOS. The files in this directory are not directly used for the custom mouse, but they provide a helpful reference for how to structure the software and use the nRF SDK and Zephyr. More information about ZMK can be found on their website: https://zmk.dev/ or their GitHub repository: https://github.com/zmkfirmware/zmk


*Disclosure* - Some of the content (notes, drivers, etc) was generated with AI assistance. To differentiate between note files for AI to read vs for humans to read, files that are all upercase like GEMINI.md are meant specifically for AI agents to have knowledge of the environment (except for README.md files). Files that are all lowercase are created for humans to read. 

## Hardware Design
The hardware design is based on the nRF52840 because of its popularity/availability, built in Bluetooth, low power consumption, and integration with the nRF SDK for software development. The PCB was designed using KiCad and currently relies on a nRF52840 nice!nano microcontroller for higher level development in the early statges. The microcontroller includes a USB-C port for charging and firmware flashing, a charging protection circuit, and enough GPIO for this project. The design also includes a PMW3389 optical sensor for mouse movement, which is supported by the nRF SDK and Zephyr.

## Software Design
The software is developed using the nRF Connect SDK, which is based on the Zephyr RTOS. The nRF Connect VS Code extension is used for development, which provides a convenient interface for firmware development. The base structure is copied directly from the nRF Desktop sample project, which provides a good starting point for developing a Bluetooth mouse. The sample provides support for Bluetooth connectivity, mouse movement, button presses, and scrolling out of the box, with additional features available to be toggled on/off using the kconfig GUI. The sample also provides sample mouse configurations as well as a PMW3360 driver, which is similar to the PMW3389. These files serve as a reference for the development of custom hardware and application files for the custom mouse. 

## Future Improvements
The next step is to design the PCB without the nice!nano which will require designing the PCB with the nRF52840 SoC directly. This will involve further researching the nRF52840 for proper implimentation as well as designing power management and charging circuits. I will also need to decide on whether to use an SoC with an included antenna or design one myself. 

On the software side, the next step is to develop custom application code for the mouse which will allow users to customize the behavior of the mouse, such as button mapping and sensitivity settings.





