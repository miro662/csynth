#ifndef __MODULES_WAVE_H
#define __MODULES_WAVE_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    float frequencyHz;
    uint32_t samplesPerSecond;
} SineWaveSettings;

void sineWaveFn(float *output, size_t outputLen, void *settings, uint64_t sampleId);

#endif