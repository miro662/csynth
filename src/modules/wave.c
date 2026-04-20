#include <math.h>
#include <stdio.h>

#include "wave.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void SineWave_Fn(float *output, size_t outputLen, void *settings, float **inputs) {
    for (size_t i = 0; i < outputLen; ++i) {
        output[i] = sin(2 * M_PI * inputs[0][i] * inputs[1][i]);
    }
}