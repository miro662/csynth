#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_timer.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "buffer.h"
#include "rack.h"
#include "modules/arith.h"
#include "modules/wave.h"

#define FREQ 44100

typedef struct {
    Rack* synth;
    Buffer* buffer;
    float* output;
    ModuleId outputModule;
} SdlStatus;

void sdlCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    (void)total_amount;

    SdlStatus *s = (SdlStatus*) userdata;

    int sample_count = additional_amount / sizeof(float);
    synthesise(s->synth, s->buffer, s->outputModule, s->output, sample_count);

    SDL_PutAudioStreamData(stream, s->output, additional_amount);
}

void sdlPlay(Rack *synth, uint32_t ms, ModuleId m) {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        exit(-1);
    }

    Buffer buffer = newBuffer();
    SdlStatus status = {
        .synth = synth,
        .buffer = &buffer,
        .output = calloc(2 << 16, sizeof(float)),
        .outputModule = m
    };

    SDL_AudioSpec audioSpec = {
        .format = SDL_AUDIO_F32,
        .channels = 1,
        .freq = FREQ
    };

    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, sdlCallback, &status);
    SDL_ResumeAudioStreamDevice(stream);

    SDL_Delay(ms);
    
    SDL_DestroyAudioStream(stream);
    SDL_Quit();
}

ModuleId buildSynth(Rack* synth);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Rack synth = newRack(64, FREQ);
    ModuleId m = buildSynth(&synth);

    sdlPlay(&synth, 5000, m);

    freeRack(synth);
    return 0;
}

ModuleId buildSynth(Rack* synth) {
    Constant_Settings *c1s = malloc(sizeof(Constant_Settings));
    c1s->value = 440.0f;
    ModuleId soundWaveFreq = addModule(synth, Constant_Fn, c1s, (uint32_t[]){0, 0});
    ModuleId soundWave = addModule(synth, SineWave_Fn, NULL, (uint32_t[]){ 0, soundWaveFreq });
    
    return soundWave;

    // Constant_Settings *c2s = malloc(sizeof(Constant_Settings));
    // c2s->value = 0.1f;
    // ModuleId timeWaveFreq = addModule(synth, Constant_Fn, c2s, (uint32_t[]){0, 0});
    // ModuleId timeWave = addModule(synth, SineWave_Fn, NULL, (uint32_t[]){ 0, timeWaveFreq });

    // return addModule(synth, Multiply_Fn, NULL, (uint32_t[]){ soundWave, timeWave });
}