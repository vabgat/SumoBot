#include "common/ring_buffer.h"

void ring_buffer_put(struct ring_buffer* rb, uint8_t data) {
    rb->buffer[rb->head] = data;
    rb->head++;
    // avoid expensive modulo operation
    // like rb->head = (rb->head + 1)% rb->size
    if (rb->head == rb->size) {
        rb->head = 0;
    }
}
uint8_t ring_buffer_get(struct ring_buffer* rb) {
    const uint8_t data = rb->buffer[rb->tail];
    rb->tail++;
    if (rb->tail == rb->size) {
        rb->tail = 0;
    }
    return data;
}
uint8_t ring_buffer_peek(const struct ring_buffer* rb) { return rb->buffer[rb->tail]; }
bool    ring_buffer_empty(const struct ring_buffer* rb) { return rb->head == rb->tail; }
bool    ring_buffer_full(const struct ring_buffer* rb) {
    uint8_t idx_after_head = rb->head + 1;
    if (idx_after_head == rb->size) {
        idx_after_head = 0;
    }
    return idx_after_head == rb->tail;
}
