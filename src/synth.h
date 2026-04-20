#ifndef __SYNTH_H
#define __SYNTH_H

#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"

typedef struct {
    bool occupied;
    float frequency;
} Channel;

typedef struct {
    Channel* channels;
    size_t chCapacity;
    size_t chSamples;
    uint64_t sample;
    uint32_t samplesPerSecond;
} Synth;
 
typedef uint32_t ChannelId;

Synth newSynth(size_t chSamples, uint64_t samplesPerSecond);
ChannelId newChannel(Synth* synth, float frequency);
void freeChannel(Synth* synth, ChannelId id);
void freeSynth(Synth synth);
void synthesise(Synth *const synth, Buffer buffer, ChannelId outChannel, float* output, size_t outSamples);

#endif