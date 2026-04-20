#include <math.h>
#include <stdio.h>

#include "wave.h"

void sineWaveFn(float *output, size_t outputLen, void *settings, uint64_t sampleId) {
    SineWaveSettings *s = (SineWaveSettings*) settings;
    for (size_t i = 0; i < outputLen; ++i) {
        float time = (float) (sampleId + i) / s->samplesPerSecond;
        output[i] = sin(2 * 3.14159 * s->frequencyHz * time);
    }
}