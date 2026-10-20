#include "byte_queue.h"
#include "critical_section.h"

void byte_queue_init(byte_queue_t *queue, uint8_t *storage, uint16_t capacity)
{
    queue->buffer   = storage;
    queue->capacity = capacity;
    queue->head     = 0U;
    queue->count    = 0U;
}

void byte_queue_clear(byte_queue_t *queue)
{
    uint32_t primask = critical_enter();
    queue->head  = 0U;
    queue->count = 0U;
    critical_exit(primask);
}

bool byte_queue_push(byte_queue_t *queue, uint8_t byte)
{
    bool stored = false;
    uint32_t primask = critical_enter();

    if (queue->count < queue->capacity) {
        uint16_t tail = (uint16_t)((queue->head + queue->count) % queue->capacity);
        queue->buffer[tail] = byte;
        queue->count++;
        stored = true;
    }

    critical_exit(primask);
    return stored;
}

bool byte_queue_peek(byte_queue_t *queue, uint8_t *byte)
{
    bool available = false;
    uint32_t primask = critical_enter();

    if (queue->count > 0U) {
        *byte = queue->buffer[queue->head];
        available = true;
    }

    critical_exit(primask);
    return available;
}

bool byte_queue_pop(byte_queue_t *queue, uint8_t *byte)
{
    bool available = false;
    uint32_t primask = critical_enter();

    if (queue->count > 0U) {
        *byte = queue->buffer[queue->head];
        queue->head = (uint16_t)((queue->head + 1U) % queue->capacity);
        queue->count--;
        available = true;
    }

    critical_exit(primask);
    return available;
}

uint16_t byte_queue_count(byte_queue_t *queue)
{
    uint32_t primask = critical_enter();
    uint16_t count = queue->count;
    critical_exit(primask);
    return count;
}
