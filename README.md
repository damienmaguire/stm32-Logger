# stm32-Logger
Welcome to the EVBMW CanLogger 6000 (Software upgradeable to the CanLogger 9000).

This project aims to create a small and cheap device for logging CAN messages in vehicle and other applications directly to an SD card and/or USB
without the need for a pc/laptop to be used and with little end user knowledge or intervention required.

- Four CAN Channels. 2 HS CAN and 2 FD/HS CAN using the MCP2518 and TCAN332.
- STM32F105 Connectivity line MCU
- USB Powered via micro USB jack
- USB communication via on chip USBCDC controller.
- Web interface over wifi for monitoring and configuration using an onboard ESP8266 Wemos module
  - Independent control of all 4 CAN channels
  - Auto/manual logging
  - Aim to have log files in SAVVYCAN format
  - All 4 can busses on an RJ45 jack allowing the use of cheap twisted pair ethernet cable for all 4 channels
  - Status and power leds onboard
- Can be powered from the vehicle using a usb adapter or via a usb power bank.

# OTA (over the air upgrade)
The firmware is linked to leave the 4 kb of flash unused. Those 4 kb are reserved for the bootloader
that you can find here: https://github.com/jsphuebner/tumanako-inverter-fw-bootloader
When flashing your device for the first time you must first flash that bootloader. After that you can
use the ESP8266 module and its web interface to upload your actual application firmware.
The web interface is here: https://github.com/jsphuebner/esp8266-web-interface

# Compiling
You will need the arm-none-eabi toolchain: https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
On Ubuntu type

`sudo apt-get install git gcc-arm-none-eabi`

The only external depedencies are libopencm3 and libopeninv. You can download and build these dependencies by typing

`make get-deps`

Now you can compile stm32-<yourname> by typing

`make`

And upload it to your board using a JTAG/SWD adapter, the updater.py script or the esp8266 web interface.

# Editing
The repository provides a project file for Code::Blocks, a rather leightweight IDE for cpp code editing.
For building though, it just executes the above command. Its build system is not actually used.
Consequently you can use your favority IDE or editor for editing files.

