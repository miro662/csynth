#ifndef __SYNTH_H
#define __SYNTH_H

#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"

typedef struct {
    bool occupied;
    float frequency;
} Module;

typedef struct {
    Module* channels;
    size_t chCapacity;
    size_t chSamples;
    uint64_t sample;
    uint32_t samplesPerSecond;
} Rack;
 
typedef uint32_t ModuleId;

Rack newRack(size_t chSamples, uint64_t samplesPerSecond);
ModuleId addModule(Rack *rack, float frequency);
void freeModule(Rack *rack, ModuleId id);
void freeRack(Rack rack);
void synthesise(Rack *const rack, Buffer *buffer, ModuleId outChannel, float *output, size_t outSamples);

#endif