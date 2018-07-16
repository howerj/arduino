# Arduino Miscellanea 

A skeleton project and collecting ground for random Arduino projects and code.

The build system has been tested under Debian, the 'arduino' package will need
to be installed and any associated tools ('avrdude', and you will also need
'picocom' to talk to the AVR).

## eForth

The current project includes a fully working eForth interpreter, whose main
limitation is lack of writable memory. Interaction takes place over the serial
port.

## Building the test program

### ATMEGA2560

To build:

	make

To flash:

	make upload

To talk to the Arduino (and the eForth interpreter running on it):

	make talk

### ATMEGA328P (Arduino Uno)

To build:

	make MCU=atmega328p

To flash:

	make upload MCU=atmega328p METHOD=arduino

To talk to the Arduino (and the eForth interpreter running on it):

	make talk

Checkout out the [makefile][] for default device setting and for which TTY is
used.

## Working platforms

* [x] ATMEGA2560
* [x] ATMEGA328P

## Projects

* Morse code encoder and decoder

Use a Morse code encoder and decoder as a front end to an eForth interpreter.

* LED light sensor and communications

A Light Emitting Diode (LED) consists of a PN junction which when hit by light
generates a small current, it is in effect a photodiode as well as a light
emitting one. This can be used to detect light levels, and even perform two way
communication over short distances (a few centimeters at a few hundred bits per
second).

See:

- <https://www.forth-ev.de/filemgmt_data/files/TR2003-35.pdf>
- <https://www.youtube.com/watch?v=vKneKz1PJCc>

[makefile]:  makefile

