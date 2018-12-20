/* LED manipulation routines
 *
 * TODO:
 * - Using an LED as a light sensor
 * - Two way communication using an LED
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
 */
#include "led.h"

unsigned led_read(led_t *l) {
	

	return 0;
}

