#include <math.h>
#include <stdio.h>

#include "wave.h"

void sineWaveFn(float *output, size_t outputLen, void *settings, float **inputs) {
    SineWaveSettings *s = (SineWaveSettings*) settings;
    for (size_t i = 0; i < outputLen; ++i) {
        output[i] = sin(2 * 3.14159 * s->frequencyHz * inputs[0][i]);
    }
}