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

ModuleId addModule(Rack* rack, float frequency) {
    // if there is a free channel, use it
    for (ModuleId i = 0; i < rack->chCapacity; i++) {
        Module* channel = &rack->channels[i];
        if (!channel->occupied) {
            channel->occupied = true;
            channel->frequency = frequency;
            return i; 
        }
    }

    // if there is no, add one member
    size_t newChCapacity = rack->chCapacity + 1;
    rack->channels = realloc(rack->channels, newChCapacity * sizeof(Module));
    ModuleId lastId = newChCapacity - 1;
    Module* lastChannel = &rack->channels[lastId];
    lastChannel->occupied = true;
    lastChannel->frequency = frequency;
    return lastId;
}

void freeModule(Rack* rack, ModuleId id) {
    rack->channels[id].occupied = false;
}

void freeRack(Rack rack) {
    free(rack.channels);
}

void synthesise(Rack *const rack, Buffer *buffer, ModuleId outChannel, float *output, size_t outSamples) {
    ensureBufferSize(buffer, rack->chCapacity * rack->chSamples);

    size_t outSample = 0;
    while (outSample < outSamples) {
        // synthesise
        for (ModuleId ch = 0; ch < rack->chCapacity; ++ch) {
            Module *chan = &rack->channels[ch];
            float *chanData = buffer->data + (ch * rack->chSamples);
            if (chan->occupied) {
                for (uint32_t s = 0; s < rack->chSamples; ++s) {
                    float time = (rack->sample + s) / (float) rack->samplesPerSecond;
                    chanData[s] = sin(time * 2 * 3.14159 * chan->frequency);
                }
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