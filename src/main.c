#include <SDL3/SDL_timer.h>

#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "rack.h"
#include "modules/arith.h"
#include "modules/wave.h"
#include "playback.h"

#define FREQ 44100

typedef struct {
    Rack* synth;
    Buffer* buffer;
    ModuleId outputModule;
} SynthContext;

void synthCallback(PlaybackBufferSpec *bufferSpec, void *userData, uint32_t samples, float *buffer) {
    (void)bufferSpec;
    SynthContext *ctx = (SynthContext*) userData;
    synthesise(ctx->synth, ctx->buffer, ctx->outputModule, buffer, samples);
}

void synthPlay(Rack *synth, uint32_t ms, ModuleId m) {
    Buffer buffer = newBuffer();
    SynthContext ctx = {
        .synth = synth,
        .buffer = &buffer,
        .outputModule = m
    };

    PlaybackThread *pt;
    Error err = playbackNew(&ctx, synthCallback, &pt);
    if (err.etype != ERR_OK) {
        fprintf(stderr, "Playback error: %s\n", err.message);
        exit(-1);
    }

    playbackResume(pt);
    SDL_Delay(ms);
    playbackFree(pt);
}

ModuleId buildSynth(Rack* synth);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Rack synth = newRack(64, FREQ);
    ModuleId m = buildSynth(&synth);

    synthPlay(&synth, 5000, m);

    freeRack(synth);
    return 0;
}

ModuleId buildSynth(Rack* synth) {
    Constant_Settings *c1s = malloc(sizeof(Constant_Settings));
    c1s->value = 440.0f;
    ModuleId soundWaveFreq = addModule(synth, Constant_Fn, c1s, (uint32_t[]){0, 0});
    ModuleId soundWave = addModule(synth, SineWave_Fn, NULL, (uint32_t[]){ 0, soundWaveFreq });

    Constant_Settings *c2s = malloc(sizeof(Constant_Settings));
    c2s->value = 0.1f;
    ModuleId timeWaveFreq = addModule(synth, Constant_Fn, c2s, (uint32_t[]){0, 0});
    ModuleId timeWave = addModule(synth, SineWave_Fn, NULL, (uint32_t[]){ 0, timeWaveFreq });

    return addModule(synth, Multiply_Fn, NULL, (uint32_t[]){ soundWave, timeWave });
}
