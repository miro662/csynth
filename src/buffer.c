#include <math.h>

#include "buffer.h"

#define PAGE_SIZE 2 << 20 // 1MB
#define MIN_SIZE 2 << 22 // 4MB

size_t _alignSize(size_t size);

/// Creates a new buffer
Buffer newBuffer() {
    return (Buffer) {
        .data = calloc(sizeof(float), MIN_SIZE),
        .len = MIN_SIZE
    };
}

/// If buffer's size is smaller than requested, grows it
void ensureBufferSize(Buffer* buffer, size_t newSize) {
    if (newSize > buffer->len) {
        buffer->len = newSize;
        // buffer->len = _alignSize(newSize);
        buffer->data = realloc(buffer->data, buffer->len);
    }
}

/// Frees buffer
void freeBuffer(Buffer buffer) {
    free(buffer.data);
}

size_t _alignSize(size_t size) {
    // align to 1mb
    size_t mb_aligned = (2 >> 20) << 20;

    // at last 4mb
    return mb_aligned < PAGE_SIZE ? PAGE_SIZE : mb_aligned;
}