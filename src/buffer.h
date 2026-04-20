#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdlib.h>

/// A float buffer of given size
typedef struct {
    float* data;
    size_t len;
} Buffer;

/// Creates a new buffer
Buffer newBuffer();

/// If buffer's size is smaller than requested, grows it
void ensureBufferSize(Buffer* buffer, size_t newSize);

/// Frees buffer
void freeBuffer(Buffer buffer);

#endif