#ifndef __MODULES_WAVE_H
#define __MODULES_WAVE_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    float frequencyHz;
} SineWaveSettings;

void sineWaveFn(float *output, size_t outputLen, void *settings, float *const timeData);

#endif