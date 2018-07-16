/* @brief Run a Forth interpreter in a limited amount of space on the arduino
 * @author Richard James Howe
 * @license MIT
 *
 * This project used the <https://github.com/howerj/embed> library, which
 * includes a Forth interpreter and a eForth image, to implement a simple
 * Forth system that communicates over the Arduino's serial connection. 
 *
 * @todo Add callbacks to manipulate the system peripherals (digital I/O,
 * analogue input, and serial).
 * @todo Allow sections of memory to be stored to flash with a 'embed_sgetc_cb'
 * @todo Yield interpreter when there is no input so we can do other work
 * @todo Speed up the interpreter, it is currently very slow. This could
 * be done by removing much of the indirection in the virtual machine,
 * and disabling the CRC check in the eForth image at start up.
 *
 * References:
 * - <https://www.arduino.cc/en/Reference/EEPROM>
 * - <https://www.arduino.cc/en/Tutorial/SoftwareSerialExample>
 * */

#include <Arduino.h>
#include <HardwareSerial.h>
#include "embed.h"

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
static const uint16_t page_4 = (EMBED_CORE_SIZE - PAGE_SIZE);

static inline bool within(cell_t range, cell_t addr) {
	return (addr >= range) && (addr < (range + PAGE_SIZE));
}

extern "C" {
	static int callback_cb(embed_t *h, void *param) {
		(void)(param);
		cell_t op = 0;
		int status = embed_pop(h, &op);
		if(status != 0)
			goto error;
		switch(op) {
		case 0: /* Pin Mode */
		{
			uint16_t pin = 0, direction = 0;
			status = embed_pop(h, &pin);
			if(status != 0)
				goto error;
			status = embed_pop(h, &direction);
			if(status != 0)
				goto error;

			Serial.write("\npin-mode: ");
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
			if(status != 0)
				goto error;

			Serial.write("\npin-read: ");
			Serial.println(pin);

			status = embed_push(h, digitalRead(pin) == HIGH ? -1 : 0);
			break;
		}
		case 2: /* Write Pin */
		{
			uint16_t pin = 0, on = 0;
			status = embed_pop(h, &pin);
			if(status != 0)
				goto error;
			status = embed_pop(h, &on);
			if(status != 0)
				goto error;

			Serial.write("\npin-set: ");
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
			if(status != 0)
				goto error;
			delay(milliseconds);
			break;
		}
		default:
			return 21;
		}

		error:
			return status;
	}

	static cell_t  rom_read_cb(embed_t const * const h, cell_t addr) {
		pages_t *p = (pages_t*)h->m;
		const uint16_t blksz = embed_default_block_size >> 1;

		if(within(page_0, addr)) {
			return p->m[0][addr];
		} else if((addr >= PAGE_SIZE) && (addr < blksz)) {
			const uint16_t naddr = addr << 1;
			const uint16_t lo    = pgm_read_byte(embed_default_block + naddr + 0);
			const uint16_t hi    = pgm_read_byte(embed_default_block + naddr + 1);
			return (hi<<8u) | lo;
		} else if(within(blksz, addr)) {
			return p->m[1][addr - blksz];
		} else if(within(page_2, addr)) {
			return p->m[2][addr - page_2];
		} else if(within(page_3, addr)) {
			return p->m[3][addr - page_3];
		} else if(within(page_4, addr)) {
			return p->m[4][addr - page_4];
		}
		return 0;
	}

	static void rom_write_cb(embed_t * const h, cell_t addr, cell_t value) {
		pages_t * const p = (pages_t*)h->m;

		const uint16_t blksz = embed_default_block_size >> 1;

		if(within(page_0, addr)) {
			p->m[0][addr] = value;
			return;
		} else if((addr >= PAGE_SIZE) && (addr < blksz)) {
			/* ROM */
		} else if(within(blksz, addr)) {
			p->m[1][addr - blksz]   = value;
			return;
		} else if(within(page_2, addr)) {
			p->m[2][addr - page_2] = value;
			return;
		} else if(within(page_3, addr)) {
			p->m[3][addr - page_3] = value;
			return;
		} else if(within(page_4, addr)) {
			p->m[4][addr - page_4] = value;
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
		/*while(Serial.write(ch) != 1)
			;*/
		Serial.write(ch);
		return ch;
	}
}

static void establish_contact(void) {
	while (Serial.available() <= 0) {
		Serial.println("FORTH SYSTEM ONLINE (AWAITING CONNECTION)");
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

	Serial.println("LOADING DEFAULT IMAGE");
	for(size_t i = 0; i < (PAGE_SIZE*2); i+=2) {
		const uint16_t lo = pgm_read_byte(embed_default_block + i + 0);
		const uint16_t hi = pgm_read_byte(embed_default_block + i + 1);
		pages.m[0][i >> 1] = (hi<<8u) | lo;
	}

	Serial.println("AWAITING USER INPUT");

	while (!Serial && (Serial.available() == 0))
		;

	Serial.println("STARTING...");
	const int r = embed_vm(&h);
	Serial.print("\nDONE (R = ");
	Serial.print(r);
	Serial.println(")");
}

