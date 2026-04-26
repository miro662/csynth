#ifndef __PLAYBACK_H
#define __PLAYBACK_H

#include <stdint.h>
#include <SDL3/SDL.h>
#include "error.h"

typedef enum {
    LAYOUT_ARRAY, LAYOUT_INTERLEAVED
} PlaybackBufferLayout;

typedef struct {
    uint32_t sampleFreq;
    uint32_t channels;
    PlaybackBufferLayout layout;
} PlaybackBufferSpec;

typedef void (*PlaybackCallback)(
    PlaybackBufferSpec *bufferSpec,
    void *userData,
    uint32_t samples,
    float *buffer
);

typedef struct {
    void * volatile userData;
    float *buffer;
    PlaybackCallback callback;
    SDL_AudioStream *audioDevice;
} PlaybackThread;

Error playbackNew(void* userData, PlaybackCallback callback, PlaybackThread **result);
void playbackResume(PlaybackThread* thread);
void playbackPause(PlaybackThread* thread);
void playbackSetUserdata(PlaybackThread* thread, void* userData);
void playbackFree(PlaybackThread* thread);

#endif
