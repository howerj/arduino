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
 * @todo Allow sections of memory to be stored to flash.
 * @todo Disable CRC check for this image to speed up the interpreter?
 * @todo Yield interpreter when there is no input so we can do other work
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

	static cell_t  rom_read_cb(embed_t const * const h, cell_t addr) {
		pages_t *p = (pages_t*)h->m;
		const uint16_t blksz = embed_default_block_size >> 1;

		if(within(page_0, addr)) {
			return p->m[0][addr];
		} else if((addr >= PAGE_SIZE) && (addr < blksz)) {
			const uint16_t naddr = addr << 1;
			const uint16_t lo    = embed_default_block[naddr+0];
			const uint16_t hi    = embed_default_block[naddr+1];
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
	h.o.write = rom_write_cb;
	h.o.options = EMBED_VM_RAW_TERMINAL;

	Serial.println("LOADING DEFAULT IMAGE");
	for(size_t i = 0; i < (PAGE_SIZE*2); i+=2) {
		const uint16_t lo = embed_default_block[i+0];
		const uint16_t hi = embed_default_block[i+1];
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

