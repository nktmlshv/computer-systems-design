#ifndef BYTE_QUEUE_H
#define BYTE_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t  *buffer;
    uint16_t  capacity;
    uint16_t  head;
    uint16_t  count;
} byte_queue_t;

void     byte_queue_init(byte_queue_t *queue, uint8_t *storage, uint16_t capacity);
void     byte_queue_clear(byte_queue_t *queue);
bool     byte_queue_push(byte_queue_t *queue, uint8_t byte);
bool     byte_queue_peek(byte_queue_t *queue, uint8_t *byte);
bool     byte_queue_pop(byte_queue_t *queue, uint8_t *byte);
uint16_t byte_queue_count(byte_queue_t *queue);

#endif
