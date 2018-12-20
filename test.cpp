/* @brief Run a Forth interpreter in a limited amount of space on the arduino
 * @author Richard James Howe
 * @license MIT
 *
 * This project used the <https://github.com/howerj/embed> library, which
 * includes a Forth interpreter and a eForth image, to implement a simple
 * Forth system that communicates over the Arduino's serial connection. The
 * eForth interpreter is extended with a series of callbacks so the hardware
 * can be quickly manipulated.
 *
 * @todo Add callbacks to manipulate the system peripherals (digital I/O,
 * analogue input, and serial).
 * @todo Allow sections of memory to be stored to flash with a 'embed_sgetc_cb'
 * @todo Yield interpreter when there is no input so we can do other work
 * @todo Speed up the interpreter, it is currently very slow. This could
 * be done by removing much of the indirection in the virtual machine,
 * and disabling the CRC check in the eForth image at start up.
 * @todo Add an Morse code decoder, the project has an encoder, but it would be
 * interesting to see if the eForth interpreter could gets its input from a
 * Morse code key, with its output going to an LED/Buzzer.
 * @todo Add code to use a simple LED for two way communications/as a light
 * sensor - an LED is a PN junction that generates a small current when hit by
 * light, this can be used as a crude light sensor and for two way
 * communication (at about 300 bits per second).
 *
 * References:
 * - <https://www.arduino.cc/en/Reference/EEPROM>
 * - <https://www.arduino.cc/en/Tutorial/SoftwareSerialExample>
 * */

#include <Arduino.h>
#include <HardwareSerial.h>
#include <avr/eeprom.h>
#include "embed.h"
#include "morse.h"

#define PAGE_SIZE (128u)
#define NPAGES    (5u)

static const int serial_baud = 9600;

typedef struct {
	cell_t m[NPAGES][PAGE_SIZE];
} pages_t;

static pages_t pages = { 0 };

static const uint16_t page_0 = 0x0000;
/*static const uint16_t page_1 = PAGE_SIZE ;*/
static const uint16_t page_2 = 0x2000;
static const uint16_t page_3 = 0x2400;

static const uint16_t page_4 = 0x4000 + (PAGE_SIZE*0);
static const uint16_t page_5 = 0x4000 + (PAGE_SIZE*1);
static const uint16_t page_6 = 0x4000 + (PAGE_SIZE*2);
static const uint16_t page_7 = 0x4000 + (PAGE_SIZE*4);

static const uint16_t page_8 = (EMBED_CORE_SIZE - PAGE_SIZE);

static inline bool within(cell_t range, cell_t addr) {
	return (addr >= range) && (addr < (range + PAGE_SIZE));
}

extern "C" {
	typedef void(*avr_reset_func)(void);
	avr_reset_func avr_reset = NULL;

	static int callback_cb(embed_t *h, void *param) {
		(void)(param);
		cell_t op = 0;
		int status = embed_pop(h, &op);
		if (status != 0)
			goto error;
		switch (op) {
		case 0: /* Pin Mode */
		{
			uint16_t pin = 0, direction = 0;
			status = embed_pop(h, &pin);
			if (status != 0)
				goto error;
			status = embed_pop(h, &direction);
			if (status != 0)
				goto error;

			Serial.write("\r\npin-mode: ");
			Serial.print(pin);
			Serial.write('/');
			Serial.println(direction);

			pinMode(pin, direction ? (direction & 0x8000 ? INPUT_PULLUP : INPUT) : OUTPUT);
			break;
		}
		case 1: /* Read Pin */
		{
			uint16_t pin = 0;
			status = embed_pop(h, &pin);
			if (status != 0)
				goto error;

			Serial.write("\r\npin-read: ");
			Serial.println(pin);

			status = embed_push(h, digitalRead(pin) == HIGH ? -1 : 0);
			break;
		}
		case 2: /* Write Pin */
		{
			uint16_t pin = 0, on = 0;
			status = embed_pop(h, &pin);
			if (status != 0)
				goto error;
			status = embed_pop(h, &on);
			if (status != 0)
				goto error;

			Serial.write("\r\npin-set: ");
			Serial.print(pin);
			Serial.write('/');
			Serial.println(on);

			digitalWrite(pin, on ? HIGH : LOW);
			break;
		}
		case 3: /* delay */
		{
			uint16_t milliseconds = 0;
			status = embed_pop(h, &milliseconds);
			if (status != 0)
				goto error;
			delay(milliseconds);
			break;
		}
		case 4: /* reset AVR */
			avr_reset();
			break;
		default:
			return 21;
		}

		error:
			return status;
	}

	static cell_t  rom_read_cb(embed_t const * const h, cell_t addr) {
		pages_t *p = (pages_t*)h->m;
		const uint16_t blksz = embed_default_block_size >> 1;

		/* RAM + ROM */
		if (within(page_0, addr)) {
			return p->m[0][addr];
		} else if ((addr >= PAGE_SIZE) && (addr < blksz)) {
			const uint16_t naddr = addr << 1;
			const uint16_t lo    = pgm_read_byte(embed_default_block + naddr + 0);
			const uint16_t hi    = pgm_read_byte(embed_default_block + naddr + 1);
			return (hi<<8u) | lo;
		} else if (within(blksz, addr)) {
			return p->m[1][addr - blksz];
		} else if (within(page_2, addr)) {
			return p->m[2][addr - page_2];
		} else if (within(page_3, addr)) {
			return p->m[3][addr - page_3];
		} else if (within(page_8, addr)) {
			return p->m[4][addr - page_8];
		}

		if (within(page_4, addr)) {
			return eeprom_read_word((uint16_t*)((addr - page_4) << 1));
		} else if (within(page_5, addr)) {
			return eeprom_read_word((uint16_t*)((addr - page_5) << 1));
		} else if (within(page_6, addr)) {
			return eeprom_read_word((uint16_t*)((addr - page_6) << 1));
		} else if (within(page_7, addr)) {
			return eeprom_read_word((uint16_t*)((addr - page_7) << 1));
		}
		
		return 0;
	}

	static void rom_write_cb(embed_t * const h, cell_t addr, cell_t value) {
		pages_t * const p = (pages_t*)h->m;

		const uint16_t blksz = embed_default_block_size >> 1;

		/* RAM + ROM */
		if (within(page_0, addr)) {
			p->m[0][addr] = value;
			return;
		} else if ((addr >= PAGE_SIZE) && (addr < blksz)) {
			/* ROM */
		} else if (within(blksz, addr)) {
			p->m[1][addr - blksz]   = value;
			return;
		} else if (within(page_2, addr)) {
			p->m[2][addr - page_2] = value;
			return;
		} else if (within(page_3, addr)) {
			p->m[3][addr - page_3] = value;
			return;
		} else if (within(page_8, addr)) {
			p->m[4][addr - page_8] = value;
			return;
		}

		/* EEPROM */
		if (within(page_4, addr)) {
			eeprom_write_word((uint16_t*)((addr - page_4) << 1), value);
			return;
		} else if (within(page_5, addr)) {
			eeprom_write_word((uint16_t*)((addr - page_5) << 1), value);
			return;
		} else if (within(page_6, addr)) {
			eeprom_write_word((uint16_t*)((addr - page_6) << 1), value);
			return;
		} else if (within(page_7, addr)) {
			eeprom_write_word((uint16_t*)((addr - page_7) << 1), value);
			return;
		}

	}

	static int serial_getc_cb(void *file, int *no_data) {
		(void)file;
		*no_data = 0;
		while (Serial.available() == 0)
			;
		return Serial.read();
	}

	static int serial_putc_cb(int ch, void *file) {
		(void)file;
		/*while (Serial.write(ch) != 1)
			;*/
		Serial.write(ch);
		return ch;
	}
}

#define UNIT_DELAY_MS (200)
#define MORSE_OUTPUT_PIN (7)

static int morse_write_char(int method, char c) {
	if (c != '.' && c != '_' && c != ' ')
		return -1;

	if (method == 0) {
		Serial.write(c);
	} else if (method == 1) {
		const int pin = MORSE_OUTPUT_PIN;
		pinMode(pin, OUTPUT);
		switch (c) {
		case '.':
			digitalWrite(pin, HIGH);
			delay(UNIT_DELAY_MS * MORSE_DOT_DELAY_MULTIPLIER);
			digitalWrite(pin, LOW);
			break;
		case '_':
			digitalWrite(pin, HIGH);
			delay(UNIT_DELAY_MS * MORSE_DASH_DELAY_MULTIPLIER);
			digitalWrite(pin, LOW);
			break;
		case ' ':
			digitalWrite(pin, LOW);
			delay(UNIT_DELAY_MS * MORSE_SPACE_DELAY_MULTIPLIER);
		}
		
	} else if (method == 2) {
		morse_write_char(0, c);
		morse_write_char(1, c);
	}
	return 1;
}

int morse_write_spaces(int method, int count) {
	for (int i = 0; i < count; i++)
		if (morse_write_char(method, ' ') < 0)
			return -1;
	return count;
}

/* 1 == LED, 0 == serial */
int morse_print_string(int method, const char *s) {
	int r = 0;
	unsigned char c = 0;
	while ((c = *s++)) {
		if (c == ' ') {
			if (morse_write_spaces(method, MORSE_SPACES_IN_WORD_SEPARATOR) < 0)
				return -1;
			r += MORSE_SPACES_IN_WORD_SEPARATOR;
		} else {
			char buffer[10] = { 0 };
			if (morse_encode_character(c, buffer, (sizeof buffer) - 1) < 0)
				return -1;
			char enc = 0;
			const char *en = buffer;
			while ((enc = *en++)) {
				if (morse_write_char(method, enc) < 0)
					return -1;
				if (morse_write_char(method, ' ') < 0)
					return -1;
				r += 2;
			}
			const int chsep = MORSE_SPACES_IN_CHAR_SEPARATOR - MORSE_SPACES_IN_ELEMENT_SEPARATOR;
			if (morse_write_spaces(method, chsep) < 0)
				return -1;
			r += chsep;
		}
	}
	return r;
}

static void establish_contact(void) {
	while (Serial.available() <= 0) {
		Serial.println("eForth: awaiting connection");
		delay(300);
	}
}

void setup(void) {
	Serial.begin(serial_baud);
	while (!Serial); 
	establish_contact();
}

void loop(void) {
	static embed_t h;
	h.o = embed_opt_default();
	h.m = &pages;
	h.o.get = serial_getc_cb;
	h.o.put = serial_putc_cb;
	h.o.read = rom_read_cb;
	h.o.callback = callback_cb;
	h.o.write = rom_write_cb;
	h.o.options = EMBED_VM_RAW_TERMINAL;

	Serial.println("loading image");
	for (size_t i = 0; i < (PAGE_SIZE*2); i+=2) {
		const uint16_t lo = pgm_read_byte(embed_default_block + i + 0);
		const uint16_t hi = pgm_read_byte(embed_default_block + i + 1);
		pages.m[0][i >> 1] = (hi<<8u) | lo;
	}

	Serial.println("(hit any key to continue)");

	while (!Serial && (Serial.available() == 0))
		;

	//morse_print_string(2, "HELLO FORTH");
	Serial.write("\r\n");
	Serial.println("starting...");
	const int r = embed_vm(&h);
	Serial.print("\r\ndone (r = ");
	Serial.print(r);
	Serial.println(")");
}

