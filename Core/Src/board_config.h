#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdbool.h>
#include "main.h"
#include "gpio_driver.h"

#define LED_GREEN_PORT        GPIOD
#define LED_GREEN_PIN         GPIO_PIN_13
#define LED_YELLOW_PORT       GPIOD
#define LED_YELLOW_PIN        GPIO_PIN_14
#define LED_RED_PORT          GPIOD
#define LED_RED_PIN           GPIO_PIN_15

#define LED_ON_LEVEL          true

#define BUTTON_PORT           GPIOC
#define BUTTON_PIN            GPIO_PIN_15
#define BUTTON_ACTIVE_LEVEL   false
#define BUTTON_PULL           PIN_PULL_UP

#endif
