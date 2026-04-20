#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_timer.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "buffer.h"
#include "synth.h"

#define FREQ 44100

typedef struct {
    Synth* synth;
    Buffer buffer;
    float* output;
} SdlStatus;

void sdlCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    SdlStatus *s = (SdlStatus*) userdata;

    int sample_count = additional_amount / sizeof(float);
    synthesise(s->synth, s->buffer, 0, s->output, sample_count);

    SDL_PutAudioStreamData(stream, s->output, additional_amount);
}

void sdlPlay(Synth *synth, uint32_t ms) {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        exit(-1);
    }

    SdlStatus status = {
        .synth = synth,
        .buffer = newBuffer(),
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

ChannelId buildSynth(Synth* synth);

int main(int argc, char **argv) {
    Synth synth = newSynth(64, FREQ);
    buildSynth(&synth);

    sdlPlay(&synth, 5000);

    freeSynth(synth);
    return 0;
}

ChannelId buildSynth(Synth* synth) {
    return newChannel(synth, 440.0);
}