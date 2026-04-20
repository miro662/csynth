#ifndef __ARITH_H
#define __ARITH_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    float value;
} Constant_Settings;

void Constant_Fn(float *output, size_t outputLen, void *settings, float **inputs);

void Add_Fn(float *output, size_t outputLen, void *settings, float **inputs);
void Multiply_Fn(float *output, size_t outputLen, void *settings, float **inputs);

#endif