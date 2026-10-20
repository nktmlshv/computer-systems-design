#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE  256U
#endif

#ifndef UART_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE  64U
#endif

#define UART_ERROR_OVERRUN  (1UL << 0U)
#define UART_ERROR_FRAMING  (1UL << 1U)
#define UART_ERROR_NOISE    (1UL << 2U)
#define UART_ERROR_PARITY   (1UL << 3U)

void     uart_driver_init(void);
void     uart_driver_process(void);

void     uart_set_interrupts(bool enabled);
bool     uart_interrupts_enabled(void);

uint16_t uart_send(const uint8_t *data, uint16_t length);
uint16_t uart_send_string(const char *text);
bool     uart_receive_byte(uint8_t *byte);

bool     uart_rx_available(void);
bool     uart_tx_idle(void);
uint32_t uart_take_errors(void);

#endif
