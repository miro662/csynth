#include "arith.h"

#include <string.h>

void Constant_Fn(float *output, size_t outputLen, void *settings, float **inputs) {
    Constant_Settings *s = (Constant_Settings*) settings;
    for (size_t i = 0; i < outputLen; ++i) {
        output[i] = s->value;
    }
}

void Add_Fn(float *output, size_t outputLen, void *settings, float **inputs) {
    for (size_t i = 0; i < outputLen; ++i) {
        output[i] = inputs[0][i] + inputs[1][i];
    }
}

void Multiply_Fn(float *output, size_t outputLen, void *settings, float **inputs) {
    for (size_t i = 0; i < outputLen; ++i) {
        output[i] = inputs[0][i] * inputs[1][i];
    }
}