#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_timer.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "buffer.h"
#include "rack.h"
#include "modules/wave.h"

#define FREQ 44100

typedef struct {
    Rack* synth;
    Buffer* buffer;
    float* output;
} SdlStatus;

void sdlCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    SdlStatus *s = (SdlStatus*) userdata;

    int sample_count = additional_amount / sizeof(float);
    synthesise(s->synth, s->buffer, 0, s->output, sample_count);

    SDL_PutAudioStreamData(stream, s->output, additional_amount);
}

void sdlPlay(Rack *synth, uint32_t ms) {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        exit(-1);
    }

    Buffer buffer = newBuffer();
    SdlStatus status = {
        .synth = synth,
        .buffer = &buffer,
        .output = calloc(2 << 16, sizeof(float))
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
    Rack synth = newRack(64, FREQ);
    buildSynth(&synth);

    sdlPlay(&synth, 5000);

    freeRack(synth);
    return 0;
}

ModuleId buildSynth(Rack* synth) {
    SineWaveSettings* svs = malloc(sizeof(SineWaveSettings));
    svs->frequencyHz = 440;
    svs->samplesPerSecond = FREQ;
    return addModule(synth, sineWaveFn, svs);
}