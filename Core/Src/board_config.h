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

#define UART_INSTANCE         USART6
#define UART_IRQ_NUMBER       USART6_IRQn
#define UART_IRQ_HANDLER      USART6_IRQHandler
#define UART_IRQ_PRIORITY     0U
#define UART_BAUD_RATE        57600U
#define UART_CLOCK_ENABLE()   (RCC->APB2ENR |= RCC_APB2ENR_USART6EN)
#define UART_CLOCK_FREQ()     HAL_RCC_GetPCLK2Freq()

#define UART_PINS_PORT        GPIOC
#define UART_PINS             (GPIO_PIN_6 | GPIO_PIN_7)
#define UART_PINS_ALT_FUNC    8U

#endif
