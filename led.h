#ifndef LED_H
#define LED_H

typedef struct {
	unsigned cathode,  /* PIN LED Cathode (-, Short Lead) is on */
		 anode;    /* PIN LED Anode   (+, Long  Lead) is on */
} led_t;

unsigned led_read(led_t *l);

#endif
