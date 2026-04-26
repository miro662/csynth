#include "playback.h"

#include <stdlib.h>
#include <SDL3/SDL.h>

void __cdecl streamCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);

Error playbackNew(void* userData, PlaybackCallback callback, PlaybackThread **result) {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        return (Error) {.etype = ERR_SDL_INIT, .message = "Cannot init SDL"};
    }

    SDL_AudioSpec audioSpec = {
        .format = SDL_AUDIO_F32,
        .channels = 1,
        .freq = 44100
    };

    PlaybackThread *pt = malloc(sizeof(PlaybackThread));
    pt->userData = userData;
    SDL_AudioStream *device = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, streamCallback, (void*) pt);
    if (device == NULL) {
        free(pt);
        return (Error) {.etype = ERR_SDL_AUDIO, .message = "Cannot open audio stream on device"};
    }
    
    pt->buffer = calloc(sizeof(float), 2 << 16);
    pt->callback = callback;
    pt->audioDevice = device;
    *result = pt;

    return (Error) {.etype = ERR_OK, .message = NULL};
}

void __cdecl streamCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    (void)total_amount;

    PlaybackThread *pt = (PlaybackThread*) userdata;
    
    SDL_AudioSpec spec;
    SDL_GetAudioStreamFormat(stream, NULL, &spec);
    PlaybackBufferSpec bufferSpec = {
        .channels = (uint32_t) spec.channels, 
        .sampleFreq = (uint32_t) spec.freq,
        .layout = LAYOUT_INTERLEAVED
    };

    int sample_count = additional_amount / sizeof(float);
    pt->callback(&bufferSpec, pt->userData, sample_count, pt->buffer);

    SDL_PutAudioStreamData(stream, pt->buffer, additional_amount);
}

void playbackResume(PlaybackThread* thread) {
    SDL_ResumeAudioStreamDevice(thread->audioDevice);
}

void playbackPause(PlaybackThread* thread) {
    SDL_PauseAudioStreamDevice(thread->audioDevice);
}

void playbackFree(PlaybackThread* thread) {
    playbackPause(thread);
    SDL_DestroyAudioStream(thread->audioDevice);
    SDL_Quit();
    free(thread->buffer);
    free(thread);
}