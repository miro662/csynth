#ifndef __SYNTH_H
#define __SYNTH_H

#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"

#define MAX_MODULE_INPUTS 2
 
typedef uint32_t ModuleId;
typedef void (*ModuleFn)(float *output, size_t outputLen, void *settings, float **inputs);

typedef enum {
    MODULE_FN,
    MODULE_TIME
} ModuleType;

typedef struct {
    bool occupied;
    ModuleType typ;
    
    ModuleFn fn;
    void *settings;
    uint32_t inputs[MAX_MODULE_INPUTS];
} Module;

typedef struct {
    Module* channels;
    ModuleId chCapacity;
    size_t chSamples;
    uint64_t sample;
    uint32_t samplesPerSecond;
} Rack;
 

Rack newRack(size_t chSamples, uint32_t samplesPerSecond);
ModuleId addModule(Rack *rack, ModuleFn fn, void *settings, uint32_t inputs[MAX_MODULE_INPUTS]);
void freeModule(Rack *rack, ModuleId id);
void freeRack(Rack rack);
void synthesise(Rack *const rack, Buffer *buffer, ModuleId outChannel, float *output, size_t outSamples);

#endif