#ifndef __SYNTH_H
#define __SYNTH_H

#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"
 
typedef void (*ModuleFn)(float *output, size_t outputLen, void *settings, uint64_t sampleId);

typedef struct {
    bool occupied;
    ModuleFn fn;
    void *settings;
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
ModuleId addModule(Rack *rack, ModuleFn fn, void *settings);
void freeModule(Rack *rack, ModuleId id);
void freeRack(Rack rack);
void synthesise(Rack *const rack, Buffer *buffer, ModuleId outChannel, float *output, size_t outSamples);

#endif