#include "rack.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

ModuleId addModuleInner(Rack *rack, ModuleType typ, ModuleFn fn, void *settings, uint32_t inputs[MAX_MODULE_INPUTS]);

Rack newRack(size_t chSamples, uint32_t samplesPerSecond) {
    ModuleId chCapacity = 16;

    Module* channels = calloc(chCapacity, sizeof(Module));

    Rack rack = {
        .channels = channels,
        .chCapacity = chCapacity,
        .chSamples = chSamples,
        .sample = 0,
        .samplesPerSecond = samplesPerSecond
    };

    addModuleInner(&rack, MODULE_TIME, NULL, NULL, (uint32_t[]){ 0, 0 });
    return rack;
};

ModuleId addModule(Rack *rack, ModuleFn fn, void *settings, uint32_t inputs[MAX_MODULE_INPUTS]) {
    return addModuleInner(rack, MODULE_FN, fn, settings, inputs);
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
        size_t remSamples = outSamples - outSample;
        size_t chunkLen = remSamples < rack->chSamples ? remSamples : rack->chSamples;

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
                        mod->fn(modOutput, chunkLen, mod->settings, inputs);
                        break;
                    case MODULE_TIME:
                        for (size_t i = 0; i < chunkLen; ++i) {
                            modOutput[i] = (float)(rack->sample + i) / rack->samplesPerSecond;
                        }
                        break;
                }
            }
        }

        // copy to destination
        float *outputPtr = buffer->data + (outChannel * rack->chSamples);
        memcpy(output + outSample, outputPtr, chunkLen * sizeof(float));
        rack->sample += chunkLen;
        outSample += chunkLen;
    }
} 

ModuleId addModuleInner(Rack *rack, ModuleType typ, ModuleFn fn, void *settings, uint32_t inputs[MAX_MODULE_INPUTS]) {
    // if there is a free channel, use it
    for (ModuleId i = 0; i < rack->chCapacity; i++) {
        Module* channel = &rack->channels[i];
        if (!channel->occupied) {
            channel->occupied = true;
            channel->fn = fn;
            channel->settings = settings;
            channel->typ = typ;
            memcpy(channel->inputs, inputs, MAX_MODULE_INPUTS * sizeof(uint32_t));
            return i; 
        }
    }

    // if there is no, add one member
    ModuleId newChCapacity = rack->chCapacity + 1;
    rack->channels = realloc(rack->channels, newChCapacity * sizeof(Module));
    ModuleId lastId = newChCapacity - 1;
    Module* lastChannel = &rack->channels[lastId];
    lastChannel->occupied = true;
    lastChannel->fn = fn;
    lastChannel->settings = settings;
    lastChannel->typ = typ;
    memcpy(lastChannel->inputs, inputs, MAX_MODULE_INPUTS * sizeof(uint32_t));
    return lastId;
}