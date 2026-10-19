#ifndef LED_H
#define LED_H

#include <stdbool.h>

typedef enum {
    LED_RED,
    LED_YELLOW,
    LED_GREEN,
    LED_COUNT
} led_t;

void led_init(void);
void led_set(led_t led, bool on);
void led_toggle(led_t led);
void led_all_off(void);

#endif
