#ifndef LED_H
#define LED_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
	unsigned tx_mark_us,
		tx_space_us,
		tx_period_us,
		rx_charge_us,
		rx_sample_us;
} led_sensor_t;

typedef enum {
	LED_MODE_EMIT_E,
	LED_MODE_REVERSE_BIAS_E,
	LED_MODE_DISCHARGE_E,
} led_mode_e;

typedef struct {
	const led_sensor_t *sensor;
	led_mode_e mode;
	unsigned cathode,  /* PIN LED Cathode (-, Short Lead) is on */
		 anode;    /* PIN LED Anode   (+, Long  Lead) is on */
} led_t;

int led_mode(led_t *l, led_mode_e mode);
int led_set(led_t *l, const int on);
unsigned led_read(led_t *l);
int led_send(led_t *l, const uint8_t b);
int led_send_string(led_t *l, const char *s);

extern const led_sensor_t led_sensor_communications; /**< Use for communications */
extern const led_sensor_t led_sensor_light_level;    /**< Use for light sensing */

#ifdef __cplusplus
}
#endif
#endif
