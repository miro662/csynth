#include "synth.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

Synth newSynth(size_t chSamples, uint64_t samplesPerSecond) {
    size_t chCapacity = 16;

    Channel* channels = calloc(chCapacity, sizeof(Channel));

    return (Synth) {
        .channels = channels,
        .chCapacity = chCapacity,
        .chSamples = chSamples,
        .sample = 0,
        .samplesPerSecond = samplesPerSecond
    };
};

ChannelId newChannel(Synth* synth, float frequency) {
    // if there is a free channel, use it
    for (ChannelId i = 0; i < synth->chCapacity; i++) {
        Channel* channel = &synth->channels[i];
        if (!channel->occupied) {
            channel->occupied = true;
            channel->frequency = frequency;
            return i; 
        }
    }

    // if there is no, add one member
    size_t newChCapacity = synth->chCapacity + 1;
    synth->channels = realloc(synth->channels, newChCapacity * sizeof(Channel));
    ChannelId lastId = newChCapacity - 1;
    Channel* lastChannel = &synth->channels[lastId];
    lastChannel->occupied = true;
    lastChannel->frequency = frequency;
    return lastId;
}

void freeChannel(Synth* synth, ChannelId id) {
    synth->channels[id].occupied = false;
}

void freeSynth(Synth synth) {
    free(synth.channels);
}

void synthesise(Synth *const synth, Buffer buffer, ChannelId outChannel, float* output, size_t outSamples) {
    ensureBufferSize(&buffer, synth->chCapacity * synth->chSamples);

    size_t outSample = 0;
    while (outSample < outSamples) {
        // synthesise
        for (ChannelId ch = 0; ch < synth->chCapacity; ++ch) {
            Channel *chan = &synth->channels[ch];
            float *chanData = buffer.data + (ch * synth->chSamples);
            if (chan->occupied) {
                for (uint32_t s = 0; s < synth->chSamples; ++s) {
                    float time = (synth->sample + s) / (float) synth->samplesPerSecond;
                    chanData[s] = sin(time * 2 * 3.14159 * chan->frequency);
                }
            }
        }
        synth->sample += synth->chSamples;

        // copy to destination
        size_t outputIdx = outChannel * synth->chSamples;
        float *outputPtr = &buffer.data[outputIdx];
        size_t remSamples = outSamples - outSample;
        size_t outputLen = remSamples < synth->chSamples ? remSamples : synth->chSamples;
        memcpy(output + outSample, outputPtr, outputLen * sizeof(float));
        outSample += outputLen;
    }
} 