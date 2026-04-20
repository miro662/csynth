#include "rack.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

ModuleId addModuleInner(Rack *rack, ModuleType moduleType, ModuleFn fn, void *settings);

Rack newRack(size_t chSamples, uint64_t samplesPerSecond) {
    size_t chCapacity = 16;

    Module* channels = calloc(chCapacity, sizeof(Module));

    Rack rack = {
        .channels = channels,
        .chCapacity = chCapacity,
        .chSamples = chSamples,
        .sample = 0,
        .samplesPerSecond = samplesPerSecond
    };
    addModuleInner(&rack, MODULE_TIME, NULL, NULL);
    return rack;
};

ModuleId addModule(Rack *rack, ModuleFn fn, void *settings) {
    return addModuleInner(rack, MODULE_FN, fn, settings);
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
            // todo: move modPtrs building logic to a better place
            float *inputs[MAX_MODULE_INPUTS];
            for (int i = 0; i < MAX_MODULE_INPUTS; ++i) {
                inputs[i] = buffer->data + (mod->inputs[i] * rack->chSamples);
            }
            if (mod->occupied) {
                switch (mod->typ) {
                    case MODULE_FN:
                        mod->fn(modOutput, rack->chSamples, mod->settings, inputs);
                        break;
                    case MODULE_TIME:    
                        for (size_t i = 0; i < rack->chSamples; ++i) {
                            modOutput[i] = (float) (rack->sample + i) / rack->samplesPerSecond;
                        }
                        break;
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

ModuleId addModuleInner(Rack *rack, ModuleType typ, ModuleFn fn, void *settings) {
    // if there is a free channel, use it
    for (ModuleId i = 0; i < rack->chCapacity; i++) {
        Module* channel = &rack->channels[i];
        if (!channel->occupied) {
            channel->occupied = true;
            channel->fn = fn;
            channel->settings = settings;
            channel->typ = typ;
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
    lastChannel->typ = typ;
    return lastId;
}