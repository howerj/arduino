/**@file led.c
 * @license MIT
 * @author Richard James Howe
 * @brief Routines for using an Light Emitting Diode as a simple
 * light *sensor*, and for bidirectional communication. This is a
 * work in progress.
 *
 * TODO:
 * - Using an LED as a light sensor
 * - Two way communication using an LED
 * - Change so this is non-blocking, a state-machine should
 *   control this.
 * - Add a PWM mode for emitting mode?
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
 * floating high/too sensitive to its surroundings).  */
#include "led.h"
#include <Arduino.h>

/* Discharge in dark a takes about 16,000 us, in bright LED light,
 * about 2000 us, that is using the LEDs and resistors that I have,
 * the system is sensitive to extra capacitances/resistances. A
 * different set of timings would be useful for sensing ambient
 * light levels as opposed to communication. These values would also
 * benefit from calibration. */

const led_sensor_t led_sensor_communications = {
	.tx_mark_us   = 4000,
	.tx_space_us  = 2000,
	.tx_period_us = 5000,
	.rx_charge_us = 200,
	.rx_sample_us = 4800,
};

const led_sensor_t led_sensor_light_level = {
	.tx_mark_us   = 1000,
	.tx_space_us  = 500,
	.tx_period_us = 5000,
	.rx_charge_us = 2000,
	.rx_sample_us = 30000,
};

int led_mode(led_t *l, led_mode_e mode) {
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

static int timer_expired(timer_t *t, unsigned long interval_in_microseconds) {
	t->current = micros();
	if ((t->current - t->prev) > (interval_in_microseconds)) {
		t->prev = t->current;
		return 1;
	}
	return 0;
}

unsigned led_read(led_t *l) {
	led_mode(l, LED_MODE_REVERSE_BIAS_E); /* charge LED */
	delayMicroseconds(l->sensor->rx_charge_us);
	led_mode(l, LED_MODE_DISCHARGE_E);
	timer_t t;
	timer_init(&t);
	while (!timer_expired(&t, l->sensor->rx_sample_us) && led_read_pin(l))
		;
	unsigned r = t.current - t.start;
	delayMicroseconds(l->sensor->rx_sample_us - r);
	return r;
}

static int led_send_bit(led_t *l, int on) {
	unsigned long delay_us = on ? l->sensor->tx_mark_us : l->sensor->tx_space_us;
	led_mode(l, LED_MODE_EMIT_E);
	delayMicroseconds(delay_us);
	led_mode(l, LED_MODE_REVERSE_BIAS_E);
	delayMicroseconds(l->sensor->tx_period_us - delay_us);
	return 0;
}

int led_send(led_t *l, uint8_t b) {
	for (size_t i = 0; i < 8; i++) {
		if (led_send_bit(l, b & 1) < 0)
			return -1;
		b >>= 1;
	}
	return 0;
}

int led_send_string(led_t *l, const char *s) {
	int ch = 0;
	while ((ch = *s++))
		if (led_send(l, ch) < 0)
			return -1;
	return 0;
}

