#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

typedef enum {
    PIN_MODE_INPUT  = 0U,
    PIN_MODE_OUTPUT = 1U,
    PIN_MODE_ALT    = 2U,
    PIN_MODE_ANALOG = 3U
} gpio_mode_t;

typedef enum {
    PIN_OTYPE_PUSH_PULL  = 0U,
    PIN_OTYPE_OPEN_DRAIN = 1U
} gpio_otype_t;

typedef enum {
    PIN_SPEED_LOW       = 0U,
    PIN_SPEED_MEDIUM    = 1U,
    PIN_SPEED_HIGH      = 2U,
    PIN_SPEED_VERY_HIGH = 3U
} gpio_speed_t;

typedef enum {
    PIN_PULL_NONE = 0U,
    PIN_PULL_UP   = 1U,
    PIN_PULL_DOWN = 2U
} gpio_pull_t;

typedef enum {
    PIN_LEVEL_LOW  = 0U,
    PIN_LEVEL_HIGH = 1U,
    PIN_LEVEL_KEEP = 2U
} gpio_level_t;

typedef struct {
    gpio_mode_t  mode;
    gpio_otype_t otype;
    gpio_speed_t speed;
    gpio_pull_t  pull;
    uint8_t      alt_func;
    gpio_level_t level;
} gpio_config_t;

void     gpio_init(GPIO_TypeDef *port, uint16_t pins, const gpio_config_t *config);
bool     gpio_get_config(GPIO_TypeDef *port, uint16_t pin, gpio_config_t *config);

uint16_t gpio_read_port(GPIO_TypeDef *port);
uint16_t gpio_read_pins(GPIO_TypeDef *port, uint16_t mask);
bool     gpio_read_pin(GPIO_TypeDef *port, uint16_t pin);

void     gpio_write_pins(GPIO_TypeDef *port, uint16_t mask, uint16_t value);
void     gpio_write_pin(GPIO_TypeDef *port, uint16_t pin, bool level);
void     gpio_toggle_pins(GPIO_TypeDef *port, uint16_t mask);

#endif
