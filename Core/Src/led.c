#include "led.h"
#include "board_config.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} led_desc_t;

static const led_desc_t leds[LED_COUNT] = {
    [LED_RED]    = { LED_RED_PORT,    LED_RED_PIN    },
    [LED_YELLOW] = { LED_YELLOW_PORT, LED_YELLOW_PIN },
    [LED_GREEN]  = { LED_GREEN_PORT,  LED_GREEN_PIN  },
};

void led_init(void)
{
    gpio_config_t config = {
        .mode     = PIN_MODE_OUTPUT,
        .otype    = PIN_OTYPE_PUSH_PULL,
        .speed    = PIN_SPEED_LOW,
        .pull     = PIN_PULL_NONE,
        .alt_func = 0U,
        .level    = LED_ON_LEVEL ? PIN_LEVEL_LOW : PIN_LEVEL_HIGH
    };

    for (int i = 0; i < LED_COUNT; i++) {
        gpio_init(leds[i].port, leds[i].pin, &config);
    }
}

void led_set(led_t led, bool on)
{
    bool level = on ? LED_ON_LEVEL : !LED_ON_LEVEL;
    gpio_write_pin(leds[led].port, leds[led].pin, level);
}

void led_toggle(led_t led)
{
    gpio_toggle_pins(leds[led].port, leds[led].pin);
}

void led_all_off(void)
{
    for (int i = 0; i < LED_COUNT; i++) {
        led_set((led_t)i, false);
    }
}
