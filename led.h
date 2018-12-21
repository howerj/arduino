#ifndef LED_H
#define LED_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
	LED_MODE_EMIT_E,
	LED_MODE_REVERSE_BIAS_E,
	LED_MODE_DISCHARGE_E,
} led_mode_e;

typedef struct {
	led_mode_e mode;
	unsigned cathode,  /* PIN LED Cathode (-, Short Lead) is on */
		 anode;    /* PIN LED Anode   (+, Long  Lead) is on */
} led_t;

int led_set(led_t *l, const int on);
unsigned long led_read(led_t *l);
int led_send(led_t *l, const uint8_t b);
int led_set_string(led_t *l, const char *s);

#ifdef __cplusplus
}
#endif
#endif
