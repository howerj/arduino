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

To build:

	make

To flash:

	make upload

To talk to the Arduino (and the eForth interpreter running on it):

	make talk

Checkout out the [makefile][] for default device setting and for which TTY is
used.

[makefile]:  makefile

