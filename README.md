# Home Alarm System

This project implements a home alarm system using an mbed LPC1768 board with an extension board that has a TextLCD display, a keypad, LEDs and switches.

## Quickstart Guide

To run this project, you need to have:

- An mbed LPC1768 board with an extension board
- A terminal program such as HyperTerminal
- A USB cable
- A power supply

Follow these steps:

1. Connect the mbed board to your computer using the USB cable.
2. Open HyperTerminal and configure it with these settings: 9600 Baud, 8 data bits, no parity bits, one stop bit and no hardware flow control.
3. Download the assignment.cpp file from this repository and copy it to your mbed drive.
4. Press the reset button on the mbed board to start the program.
5. Follow the instructions on the TextLCD display and use the keypad to interact with the system.

## Installation Steps

If you want to modify or compile this project yourself, you need to have:

- An mbed account
- An online compiler or an offline toolchain
- The TextLCD library

Follow these steps:

1. Log in to your mbed account and import this project from this repository.
2. Add the TextLCD library from https://os.mbed.com/users/wim/code/TextLCD/
3. Edit the assignment.cpp file as you wish or write your own code.
4. Compile the project online or offline and download the binary file to your mbed drive.

## APIs

This project uses some APIs from mbed OS and TextLCD library. Here are some examples:

- `TextLCD lcd(p15,p16,p17,p18,p19,p20);` creates a TextLCD object with pins for rs,e,d4-d7
- `BusOut rows(p26,p25,p24);` creates a BusOut object with pins for rows of keypad
- `BusIn cols(p14,p13,p12,p11);` creates a BusIn object with pins for columns of keypad
- `SPI sw(p5,p6,p7);` creates an SPI object with pins for mosi,miso,sclk of shift register
- `DigitalOut cs(p8);` creates a DigitalOut object with pin for cs of shift register
- `DigitalOut boardLed(LED1);` creates a DigitalOut object with pin for LED1 on board
- `Ticker myTicker;` creates a Ticker object for periodic actions
- `Timer exitPeriod;` creates a Timer object for exit period countdown
- `lcd.printf("Hello World!");` prints "Hello World!" on LCD screen
- `rows = i;` sets row i of keypad high 
- `char b = getKey();` gets pressed key from keypad 
- `sw.write(order);` writes order value to shift register 
- `cs = 1; cs = 0;` latches order value to LEDs 
- `myTicker.attach(&ledCall , 0.5f);` attaches ledCall function to ticker every 0.5 seconds 
- `exitPeriod.start(); exitPeriod.stop(); exitPeriod.reset(); exitPeriod.read();`
starts,stops,resets,and reads timer value 

For more details about these APIs, please refer to https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html and https://os.mbed.com/users/wim/code/TextLCD/docs/tip/classTextLCD.html

## License

This project do not have license because I do not know how to get it... hahaha
