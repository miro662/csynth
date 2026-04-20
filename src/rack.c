#include "rack.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

Rack newRack(size_t chSamples, uint64_t samplesPerSecond) {
    size_t chCapacity = 16;

    Module* channels = calloc(chCapacity, sizeof(Module));

    return (Rack) {
        .channels = channels,
        .chCapacity = chCapacity,
        .chSamples = chSamples,
        .sample = 0,
        .samplesPerSecond = samplesPerSecond
    };
};

ModuleId addModule(Rack *rack, ModuleFn fn, void *settings) {
    // if there is a free channel, use it
    for (ModuleId i = 0; i < rack->chCapacity; i++) {
        Module* channel = &rack->channels[i];
        if (!channel->occupied) {
            channel->occupied = true;
            channel->fn = fn;
            channel->settings = settings;
            // channel->deps is 0, correct for now
            return i; 
        }
    }

    // if there is no, add one member
    size_t newChCapacity = rack->chCapacity + 1;
    rack->channels = realloc(rack->channels, newChCapacity * sizeof(Module));
    ModuleId lastId = newChCapacity - 1;
    Module* lastChannel = &rack->channels[lastId];
    lastChannel->occupied = true;
    lastChannel->fn = fn;
    lastChannel->settings = settings;
    // channel->deps is 0, correct for now
    return lastId;
}

void freeModule(Rack* rack, ModuleId id) {
    rack->channels[id].occupied = false;
    if (rack->channels[id].settings != NULL) {
        free(rack->channels[id].settings);
    }
}

void freeRack(Rack rack) {
    for (size_t ch = 0; ch < rack.chCapacity; ++ch) {
        if (rack.channels[ch].occupied && rack.channels[ch].settings != NULL) {
            free(rack.channels[ch].settings);
        }
    }
    free(rack.channels);
}

void synthesise(Rack *const rack, Buffer *buffer, ModuleId outChannel, float *output, size_t outSamples) {
    ensureBufferSize(buffer, rack->chCapacity * rack->chSamples);

    size_t outSample = 0;
    while (outSample < outSamples) {
        // synthesise
        for (ModuleId m = 0; m < rack->chCapacity; ++m) {
            Module *mod = &rack->channels[m];
            float *modOutput = buffer->data + (m * rack->chSamples);
            if (mod->occupied) {
                mod->fn(modOutput, rack->chSamples, mod->settings, rack->sample);
            }
        }
        rack->sample += rack->chSamples;

        // copy to destination
        size_t outputIdx = outChannel * rack->chSamples;
        float *outputPtr = &buffer->data[outputIdx];
        size_t remSamples = outSamples - outSample;
        size_t outputLen = remSamples < rack->chSamples ? remSamples : rack->chSamples;
        memcpy(output + outSample, outputPtr, outputLen * sizeof(float));
        outSample += outputLen;
    }
} 