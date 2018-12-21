/* LED manipulation routines
 *
 * TODO:
 * - Using an LED as a light sensor
 * - Two way communication using an LED
 * - Change so this is non-blocking, a state-machine should
 *   control this.
 *
 *
 *
 * This LED controller uses two I/O pins so an LED can be used
 * as both an output, and as a light sensor.
 *           
 *               Pin 1    Anode  Cathode                   Pin 2
 * Emitting:     +5v   --- (+) LED (-) ---- Resistor (1k) -- 0v
 * Reverse Bias:  0V   --- (+) LED (-) ---- Resistor (1k) -- 5v
 * Discharge:     0v   --- (+) LED (-) ---- Resistor (1k) -- IN
 *
 * *Circuit might need changing (input impedance too high/or input
 * floating high).
 *
 */
#include "led.h"
#include <Arduino.h>

static int led_mode(led_t *l, led_mode_e mode) {
	switch (mode) {
	case LED_MODE_EMIT_E:
		pinMode(l->anode,   OUTPUT);
		pinMode(l->cathode, OUTPUT);
		digitalWrite(l->anode,   HIGH);
		digitalWrite(l->cathode, LOW);
		break;
	case LED_MODE_REVERSE_BIAS_E:
		pinMode(l->anode,   OUTPUT);
		pinMode(l->cathode, OUTPUT);
		digitalWrite(l->anode,   LOW);
		digitalWrite(l->cathode, HIGH);
		break;
	case LED_MODE_DISCHARGE_E:
		pinMode(l->anode,   OUTPUT);
		pinMode(l->cathode, INPUT /*INPUT_PULLUP*/);
		digitalWrite(l->anode, LOW);
		break;
	default:
		return -1;
	}
	l->mode = mode;
	return 0;
}


int led_set(led_t *l, const int on) {
	return led_mode(l, on ? LED_MODE_EMIT_E : LED_MODE_REVERSE_BIAS_E);
}

static int led_read_pin(led_t *l) {
	if (l->mode != LED_MODE_DISCHARGE_E)
		return -1;
	return digitalRead(l->cathode) == HIGH ? 1 : 0;
}

typedef struct {
	unsigned long start, prev, current;
} timer_t;

static int timer_init(timer_t *t) {
	t->prev    = micros();
	t->start   = t->prev;
	t->current = t->prev;
	return 0;
}

static int timer_expired(timer_t *t, unsigned long interval_in_milliseconds) {
	t->current = micros();
	if ((t->current - t->prev) > (1000uL * interval_in_milliseconds)) {
		t->prev = t->current;
		return 1;
	}
	return 0;
}

unsigned long led_read(led_t *l) {
	led_mode(l, LED_MODE_REVERSE_BIAS_E); /* charge LED */
	delay(40 /* 1 ms */);
	led_mode(l, LED_MODE_DISCHARGE_E);
	timer_t t;
	timer_init(&t);
	int r;
	while (!timer_expired(&t, 40 /* ms */) && (r = led_read_pin(l)))
		;
	led_mode(l, LED_MODE_EMIT_E);
	//delay(10);
	return t.current - t.start;
}

