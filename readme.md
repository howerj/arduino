# Arduino Miscellanea 

A skeleton project and collecting ground for random [Arduino][] projects and code.

The build system has been tested under Debian, the 'arduino' package will need
to be installed and any associated tools ('avrdude', and you will also need
'picocom' to talk to the AVR).

## eForth

The current project includes a fully working [eForth][] interpreter, whose main
limitation is lack of writable memory. Interaction takes place over the serial
port. The interpreter is currently quite slow but could be sped up by removing
a lot of the indirection.

## Building the test program

### ATMEGA2560

To build:

	make MCU=atmega2560

To flash:

	make upload MCU=atmega2560 METHOD=wiring

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

**The projects section is a work in progress!**

These projects are currently works in progress, they either have not been
implemented or only implemented partially.

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

## To-Do

* [ ] Improve eForth system
  * [x] Get eForth system up and running
  * [ ] Speed up interpreter; This can be done by removing a lot of the
  indirection.
  * [ ] Allow saving/loading of dictionary to EEPROM.
  * [ ] Maximize usable memory depending on platform.
  * [ ] Create custom eForth image that includes extension words in system
  instead of extending the system at run time.
  * [ ] Add words for interacting with all the peripherals
* [ ] Implement LED light sensor and communications module
  * [ ] Test out different types of LEDs
  * [ ] Get two way communication working
  * [ ] Document system, make a write up describing how it works
* [ ] Implement Morse Code CODEC
  * [x] Implement Encoder
  * [ ] Implement Decoder
  * [ ] Document CODEC

[makefile]:  makefile
[eForth]: https://github.com/howerj/embed
[Arduino]: https://www.arduino.cc/

