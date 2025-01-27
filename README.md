# Arcade Bluetooth Controller Adapter "BlueJamma"

## Overview
This project provides a hardware and software solution to add Bluetooth gamepad support to Jamma/Chamma arcade boards (like Pandora Box), while maintaining the original physical controls functionality. The adapter allows connecting up to 2 Bluetooth gamepads simultaneously with the existing control panel.

The device is built around the ESP32-C3 SuperMini development board, which provides an excellent balance of size, features, and cost. The control program is based on the BluePad32 library and can be found in the `sketch` folder of this repository.

## Features
- Supports up to 2 Bluetooth gamepads simultaneously
- Maintains original control panel functionality
- Compatible with Jamma/Chamma arcade boards
- Easy installation and setup
- Open-source hardware and software

## Hardware
The project uses the ESP32-C3 SuperMini form factor for its compact size and built-in USB capabilities. A complete list of required components and materials can be found in the [Bill of Materials](bom/BOM.md).

The PCB design incorporates elements from the [JammaKludge project](https://github.com/estechnical/jammakludge) by estechnical, which provided valuable reference for the Jamma connector implementation.

## Demonstration

### PCB Design
<img src="images/pcb_render.jpg" width="400" alt="PCB Front Render">
<img src="images/pcb_back_render.jpg" width="400" alt="PCB Back Render">

### Enclosure Design
Check out the 3D render of the enclosure design:
https://github.com/KPEKEP/BlueJamma/raw/main/enclosure/24v.avi

STEP files for the enclosure are available in the [enclosure/export/rc2/](enclosure/export/rc2/) folder.

### Assembled Device
<img src="images/pcb.jpg" width="400" alt="Manufactured PCB">
<img src="images/pcb_back.jpg" width="400" alt="Manufactured PCB Back">
<img src="images/soldered.jpg" width="400" alt="Soldered Device">
Assembled device: https://github.com/KPEKEP/BlueJamma/raw/main/images/assembled.MOV

Watch a demo of the device working with Bluetooth controllers:
https://github.com/KPEKEP/BlueJamma/raw/main/images/demo.mp4

## Known Issues

### Bottom Mounting Issue
The current PCB design lacks proper plastic caps to hold screws in place on the bottom side of the board. This can make mounting slightly inconvenient.

<img src="images/issue1.jpg" width="400" alt="Bottom mounting issue">

Status: Fix planned for the next PCB revision.

## Disclaimer and License

### Non-Commercial Use License
This project is released under a Non-Commercial Use License. You are free to:
- Use this project for personal, non-commercial purposes
- Modify and adapt the code for personal use
- Share the project with others, provided you maintain the same license

You may NOT:
- Use this project for commercial purposes
- Sell products based on this project
- Include this project in commercial applications

### Risk Disclaimer
**USE AT YOUR OWN RISK**: The hardware and software in this project are provided "as is", without warranty of any kind, express or implied. The creators and contributors of this project accept no responsibility or liability for any damage, injury, or loss resulting from the use of this project's hardware, software, or documentation.

- We are not responsible for any damage to your arcade board, control panel, or other equipment
- Modify your hardware at your own risk
- Always verify connections and power requirements before use

### Components Attribution
This project uses:
- Open-source libraries and components, all properly credited in the source code
- Originally written code and hardware designs
- Third-party libraries used under their respective licenses (see Dependencies section)

## Contributing
Contributions are welcome! Please read our contributing guidelines before submitting pull requests.

## Support
This is a community project. For support:
- Open an issue on GitHub
- Check existing documentation
- Join our community discussions

## Authors and Acknowledgment
Pavel Nakaznenko, 2025

This project relies on and gratefully acknowledges the following tools and projects:

- [BluePad32](https://github.com/ricardoquesada/bluepad32) - The excellent Bluetooth gamepad support library that powers the controller functionality
- [ESP32-C3](https://www.espressif.com/en/products/socs/esp32-c3) - The RISC-V based microcontroller from Espressif
- [Arduino IDE](https://www.arduino.cc/en/software) - The development environment used for programming
- [KiCad](https://www.kicad.org/) - The open-source software used for electronic design
- [JLCPCB](https://jlcpcb.com/) - PCB manufacturing and assembly services
- [JammaKludge](https://github.com/estechnical/jammakludge) - Reference materials for Jamma connector implementation

Special thanks to all the open-source contributors and communities behind these projects.

---

Remember: This project is for non-commercial use only. All modifications and usage are at your own risk. 

LICENSE: [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/)