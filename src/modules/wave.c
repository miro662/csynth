#include <math.h>
#include <stdio.h>

#include "wave.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void SineWave_Fn(float *output, size_t outputLen, void *settings, float **inputs) {
    (void)settings;

    for (size_t i = 0; i < outputLen; ++i) {
        output[i] = sinf(2.0f * M_PI * inputs[0][i] * inputs[1][i]);
    }
}