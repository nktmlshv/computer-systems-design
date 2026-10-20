#include "uart_driver.h"
#include "board_config.h"
#include "byte_queue.h"
#include "critical_section.h"
#include <string.h>

#define RX_BAD_FRAME_FLAGS  (USART_SR_PE | USART_SR_FE | USART_SR_NE)

static uint8_t      tx_storage[UART_TX_BUFFER_SIZE];
static uint8_t      rx_storage[UART_RX_BUFFER_SIZE];
static byte_queue_t tx_queue;
static byte_queue_t rx_queue;

static volatile bool     interrupts_enabled;
static volatile uint32_t error_flags;

static void configure_pins(void)
{
    gpio_config_t config = {
        .mode     = PIN_MODE_ALT,
        .otype    = PIN_OTYPE_PUSH_PULL,
        .speed    = PIN_SPEED_VERY_HIGH,
        .pull     = PIN_PULL_NONE,
        .alt_func = UART_PINS_ALT_FUNC,
        .level    = PIN_LEVEL_KEEP
    };
    gpio_init(UART_PINS_PORT, UART_PINS, &config);
}

static uint32_t baud_divider(void)
{
    return (UART_CLOCK_FREQ() + (UART_BAUD_RATE / 2U)) / UART_BAUD_RATE;
}

static void configure_registers(void)
{
    UART_INSTANCE->CR1 = 0U;
    UART_INSTANCE->CR2 = 0U;
    UART_INSTANCE->CR3 = 0U;
    UART_INSTANCE->BRR = baud_divider();
    UART_INSTANCE->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

    (void)UART_INSTANCE->SR;
    (void)UART_INSTANCE->DR;
}

static void record_errors(uint32_t status)
{
    if ((status & USART_SR_ORE) != 0U) {
        error_flags |= UART_ERROR_OVERRUN;
    }
    if ((status & USART_SR_FE) != 0U) {
        error_flags |= UART_ERROR_FRAMING;
    }
    if ((status & USART_SR_NE) != 0U) {
        error_flags |= UART_ERROR_NOISE;
    }
    if ((status & USART_SR_PE) != 0U) {
        error_flags |= UART_ERROR_PARITY;
    }
}

static void receive_pending(void)
{
    uint32_t status = UART_INSTANCE->SR;

    if ((status & (USART_SR_RXNE | USART_SR_ORE)) == 0U) {
        return;
    }

    uint8_t byte = (uint8_t)UART_INSTANCE->DR;
    record_errors(status);

    if (((status & USART_SR_RXNE) != 0U) && ((status & RX_BAD_FRAME_FLAGS) == 0U)) {
        (void)byte_queue_push(&rx_queue, byte);
    }
}

static bool transmit_next(void)
{
    uint8_t byte;

    if (((UART_INSTANCE->SR & USART_SR_TXE) != 0U) &&
        byte_queue_pop(&tx_queue, &byte)) {
        UART_INSTANCE->DR = byte;
        return true;
    }
    return false;
}

void UART_IRQ_HANDLER(void)
{
    receive_pending();

    if (((UART_INSTANCE->CR1 & USART_CR1_TXEIE) != 0U) &&
        ((UART_INSTANCE->SR & USART_SR_TXE) != 0U)) {
        if (!transmit_next()) {
            UART_INSTANCE->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}

void uart_driver_init(void)
{
    byte_queue_init(&tx_queue, tx_storage, UART_TX_BUFFER_SIZE);
    byte_queue_init(&rx_queue, rx_storage, UART_RX_BUFFER_SIZE);

    UART_CLOCK_ENABLE();
    configure_pins();
    configure_registers();

    NVIC_SetPriority(UART_IRQ_NUMBER, UART_IRQ_PRIORITY);
    uart_set_interrupts(true);
}

void uart_set_interrupts(bool enabled)
{
    uint32_t primask = critical_enter();

    if (enabled) {
        interrupts_enabled = true;
        UART_INSTANCE->CR1 |= USART_CR1_RXNEIE;
        if (byte_queue_count(&tx_queue) > 0U) {
            UART_INSTANCE->CR1 |= USART_CR1_TXEIE;
        }
        NVIC_EnableIRQ(UART_IRQ_NUMBER);
    } else {
        interrupts_enabled = false;
        UART_INSTANCE->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_TXEIE);
        NVIC_DisableIRQ(UART_IRQ_NUMBER);
        NVIC_ClearPendingIRQ(UART_IRQ_NUMBER);
    }

    critical_exit(primask);
}

bool uart_interrupts_enabled(void)
{
    return interrupts_enabled;
}

void uart_driver_process(void)
{
    if (!interrupts_enabled) {
        receive_pending();
        (void)transmit_next();
    }
}

uint16_t uart_send(const uint8_t *data, uint16_t length)
{
    uint16_t queued = 0U;

    while ((queued < length) && byte_queue_push(&tx_queue, data[queued])) {
        queued++;
    }

    if (interrupts_enabled && (queued > 0U)) {
        uint32_t primask = critical_enter();
        UART_INSTANCE->CR1 |= USART_CR1_TXEIE;
        critical_exit(primask);
    }
    return queued;
}

uint16_t uart_send_string(const char *text)
{
    return uart_send((const uint8_t *)text, (uint16_t)strlen(text));
}

bool uart_receive_byte(uint8_t *byte)
{
    return byte_queue_pop(&rx_queue, byte);
}

bool uart_rx_available(void)
{
    return byte_queue_count(&rx_queue) > 0U;
}

bool uart_tx_idle(void)
{
    return (byte_queue_count(&tx_queue) == 0U) &&
           ((UART_INSTANCE->SR & USART_SR_TC) != 0U);
}

uint32_t uart_take_errors(void)
{
    uint32_t primask = critical_enter();
    uint32_t flags = error_flags;
    error_flags = 0U;
    critical_exit(primask);
    return flags;
}
