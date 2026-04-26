#include <math.h>

#include "buffer.h"

size_t _alignSize(size_t size);

/// Creates a new buffer
Buffer newBuffer() {
    size_t size = _alignSize(0);
    return (Buffer) {
        .data = calloc(sizeof(float), size),
        .len = size
    };
}

/// If buffer's size is smaller than requested, grows it
void ensureBufferSize(Buffer* buffer, size_t newSize) {
    if (newSize > buffer->len) {
        buffer->len = _alignSize(newSize);
        buffer->data = realloc(buffer->data, buffer->len);
    }
}

/// Frees buffer
void freeBuffer(Buffer buffer) {
    free(buffer.data);
}

size_t _alignSize(size_t size) {
    const size_t SHIFT = 20;
    size_t megabytes = max(size >> SHIFT, 1);
    return megabytes << SHIFT;
}