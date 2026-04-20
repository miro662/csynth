#ifndef __MODULES_WAVE_H
#define __MODULES_WAVE_H

#include <stdint.h>
#include <stdlib.h>

void SineWave_Fn(float *output, size_t outputLen, void *settings, float **inputs);

#endif