#include "gpio_driver.h"
#include <stddef.h>

static bool is_single_pin(uint16_t pin)
{
    return (pin != 0U) && ((pin & (uint16_t)(pin - 1U)) == 0U);
}

static uint32_t pin_number(uint16_t pin)
{
    uint32_t n = 0U;
    while (((uint32_t)pin >> n) != 1U) {
        n++;
    }
    return n;
}

static void clock_enable(GPIO_TypeDef *port)
{
    uint32_t bit = 1UL << (((uint32_t)port - (uint32_t)GPIOA_BASE) >> 10U);
#if defined(RCC_AHBENR_GPIOAEN)
    RCC->AHBENR |= bit;
    (void)RCC->AHBENR;
#else
    RCC->AHB1ENR |= bit;
    (void)RCC->AHB1ENR;
#endif
}

static void set_fields(volatile uint32_t *reg, uint32_t pins, uint32_t width, uint32_t value)
{
    uint32_t field = (1UL << width) - 1UL;
    uint32_t mask  = 0U;
    uint32_t bits  = 0U;

    for (uint32_t i = 0U; i < (32U / width); i++) {
        if ((pins & (1UL << i)) != 0U) {
            mask |= field << (i * width);
            bits |= (value & field) << (i * width);
        }
    }
    *reg = (*reg & ~mask) | bits;
}

static void write_masked(GPIO_TypeDef *port, uint16_t mask, uint16_t value)
{
    uint32_t set   = (uint32_t)(value & mask);
    uint32_t reset = (uint32_t)((uint16_t)(~value) & mask);
    port->BSRR = (reset << 16U) | set;
}

void gpio_init(GPIO_TypeDef *port, uint16_t pins, const gpio_config_t *config)
{
    if ((port == NULL) || (pins == 0U) || (config == NULL)) {
        return;
    }

    clock_enable(port);

    gpio_pull_t pull = (config->mode == PIN_MODE_ANALOG) ? PIN_PULL_NONE : config->pull;

    if ((config->mode == PIN_MODE_OUTPUT) && (config->level != PIN_LEVEL_KEEP)) {
        write_masked(port, pins, (config->level == PIN_LEVEL_HIGH) ? pins : (uint16_t)0U);
    }

    if ((config->mode == PIN_MODE_OUTPUT) || (config->mode == PIN_MODE_ALT)) {
        set_fields(&port->OTYPER,  pins, 1U, (uint32_t)config->otype);
        set_fields(&port->OSPEEDR, pins, 2U, (uint32_t)config->speed);
    }

    set_fields(&port->PUPDR, pins, 2U, (uint32_t)pull);

    if (config->mode == PIN_MODE_ALT) {
        set_fields(&port->AFR[0], (uint32_t)pins & 0x00FFU, 4U, config->alt_func);
        set_fields(&port->AFR[1], (uint32_t)pins >> 8U,     4U, config->alt_func);
    }

    set_fields(&port->MODER, pins, 2U, (uint32_t)config->mode);
}

bool gpio_get_config(GPIO_TypeDef *port, uint16_t pin, gpio_config_t *config)
{
    if ((port == NULL) || (config == NULL) || !is_single_pin(pin)) {
        return false;
    }

    uint32_t n = pin_number(pin);

    config->mode     = (gpio_mode_t)((port->MODER >> (2U * n)) & 0x3U);
    config->otype    = (gpio_otype_t)((port->OTYPER >> n) & 0x1U);
    config->speed    = (gpio_speed_t)((port->OSPEEDR >> (2U * n)) & 0x3U);
    config->pull     = (gpio_pull_t)((port->PUPDR >> (2U * n)) & 0x3U);
    config->alt_func = (uint8_t)((port->AFR[n >> 3U] >> ((n & 7U) * 4U)) & 0xFU);
    config->level    = (((port->ODR >> n) & 0x1U) != 0U) ? PIN_LEVEL_HIGH : PIN_LEVEL_LOW;
    return true;
}

uint16_t gpio_read_port(GPIO_TypeDef *port)
{
    return (uint16_t)port->IDR;
}

uint16_t gpio_read_pins(GPIO_TypeDef *port, uint16_t mask)
{
    return (uint16_t)(port->IDR & mask);
}

bool gpio_read_pin(GPIO_TypeDef *port, uint16_t pin)
{
    return (port->IDR & pin) != 0U;
}

void gpio_write_pins(GPIO_TypeDef *port, uint16_t mask, uint16_t value)
{
    write_masked(port, mask, value);
}

void gpio_write_pin(GPIO_TypeDef *port, uint16_t pin, bool level)
{
    write_masked(port, pin, level ? pin : (uint16_t)0U);
}

void gpio_toggle_pins(GPIO_TypeDef *port, uint16_t mask)
{
    uint32_t odr = port->ODR;
    uint32_t m   = (uint32_t)mask;
    port->BSRR = ((odr & m) << 16U) | (~odr & m);
}
