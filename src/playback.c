#include "playback.h"

#include <stdlib.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

static const CLSID sCLSID_MMDeviceEnumerator =
    {0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}};
static const IID sIID_IMMDeviceEnumerator =
    {0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}};
static const IID sIID_IAudioClient =
    {0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2}};
static const IID sIID_IAudioRenderClient =
    {0xF294ACFC, 0x3146, 0x4483, {0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2}};

static DWORD WINAPI audioThread(LPVOID param) {
    PlaybackThread *pt = (PlaybackThread *)param;
    IAudioClient *ac = (IAudioClient *)pt->audioClient;
    IAudioRenderClient *rc = (IAudioRenderClient *)pt->renderClient;
    HANDLE handles[2] = { (HANDLE)pt->hEvent, (HANDLE)pt->hStopEvent };

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    while (1) {
        DWORD result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        if (result == WAIT_OBJECT_0 + 1 || result == WAIT_FAILED) break;

        UINT32 padding;
        if (FAILED(ac->lpVtbl->GetCurrentPadding(ac, &padding))) continue;

        UINT32 available = pt->bufferFrames - padding;
        if (available == 0) continue;

        BYTE *pData;
        if (FAILED(rc->lpVtbl->GetBuffer(rc, available, &pData))) continue;

        void *ud = pt->userData;
        PlaybackBufferSpec spec = {
            .sampleFreq = pt->sampleRate,
            .channels = 1,
            .layout = LAYOUT_INTERLEAVED
        };
        pt->callback(&spec, ud, available, (float *)pData);

        rc->lpVtbl->ReleaseBuffer(rc, available, 0);
    }
    return 0;
}

Error playbackNew(void *userData, PlaybackCallback callback, PlaybackThread **result) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return (Error){.etype = ERR_WASAPI_INIT, .message = "Cannot initialize COM"};
    }
    int comInitialized = SUCCEEDED(hr);

    IMMDeviceEnumerator *pEnumerator = NULL;
    hr = CoCreateInstance(&sCLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                          &sIID_IMMDeviceEnumerator, (void **)&pEnumerator);
    if (FAILED(hr)) {
        if (comInitialized) CoUninitialize();
        return (Error){.etype = ERR_WASAPI_INIT, .message = "Cannot create device enumerator"};
    }

    IMMDevice *pDevice = NULL;
    hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator, eRender, eConsole, &pDevice);
    pEnumerator->lpVtbl->Release(pEnumerator);
    if (FAILED(hr)) {
        if (comInitialized) CoUninitialize();
        return (Error){.etype = ERR_WASAPI_INIT, .message = "Cannot get default audio endpoint"};
    }

    IAudioClient *pAudioClient = NULL;
    hr = pDevice->lpVtbl->Activate(pDevice, &sIID_IAudioClient, CLSCTX_ALL, NULL, (void **)&pAudioClient);
    pDevice->lpVtbl->Release(pDevice);
    if (FAILED(hr)) {
        if (comInitialized) CoUninitialize();
        return (Error){.etype = ERR_WASAPI_AUDIO, .message = "Cannot activate audio client"};
    }

    REFERENCE_TIME defaultPeriod, minPeriod;
    pAudioClient->lpVtbl->GetDevicePeriod(pAudioClient, &defaultPeriod, &minPeriod);

    WAVEFORMATEX wfx = {
        .wFormatTag = WAVE_FORMAT_IEEE_FLOAT,
        .nChannels = 1,
        .nSamplesPerSec = 44100,
        .nAvgBytesPerSec = 44100 * sizeof(float),
        .nBlockAlign = sizeof(float),
        .wBitsPerSample = 32,
        .cbSize = 0
    };

    hr = pAudioClient->lpVtbl->Initialize(pAudioClient,
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
        AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
        2 * defaultPeriod, 0, &wfx, NULL);
    if (FAILED(hr)) {
        pAudioClient->lpVtbl->Release(pAudioClient);
        if (comInitialized) CoUninitialize();
        return (Error){.etype = ERR_WASAPI_AUDIO, .message = "Cannot initialize audio client"};
    }

    UINT32 bufferFrames;
    pAudioClient->lpVtbl->GetBufferSize(pAudioClient, &bufferFrames);

    IAudioRenderClient *pRenderClient = NULL;
    hr = pAudioClient->lpVtbl->GetService(pAudioClient, &sIID_IAudioRenderClient, (void **)&pRenderClient);
    if (FAILED(hr)) {
        pAudioClient->lpVtbl->Release(pAudioClient);
        if (comInitialized) CoUninitialize();
        return (Error){.etype = ERR_WASAPI_AUDIO, .message = "Cannot get render client"};
    }

    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hEvent || !hStopEvent) {
        if (hEvent) CloseHandle(hEvent);
        if (hStopEvent) CloseHandle(hStopEvent);
        pRenderClient->lpVtbl->Release(pRenderClient);
        pAudioClient->lpVtbl->Release(pAudioClient);
        if (comInitialized) CoUninitialize();
        return (Error){.etype = ERR_WASAPI_AUDIO, .message = "Cannot create events"};
    }

    pAudioClient->lpVtbl->SetEventHandle(pAudioClient, hEvent);

    PlaybackThread *pt = malloc(sizeof(PlaybackThread));
    pt->userData = userData;
    pt->buffer = NULL;
    pt->callback = callback;
    pt->audioClient = pAudioClient;
    pt->renderClient = pRenderClient;
    pt->hEvent = hEvent;
    pt->hStopEvent = hStopEvent;
    pt->bufferFrames = bufferFrames;
    pt->sampleRate = 44100;
    pt->comInitialized = comInitialized;

    HANDLE hThread = CreateThread(NULL, 0, audioThread, pt, 0, NULL);
    if (!hThread) {
        CloseHandle(hEvent);
        CloseHandle(hStopEvent);
        pRenderClient->lpVtbl->Release(pRenderClient);
        pAudioClient->lpVtbl->Release(pAudioClient);
        if (comInitialized) CoUninitialize();
        free(pt);
        return (Error){.etype = ERR_WASAPI_AUDIO, .message = "Cannot create render thread"};
    }
    pt->hThread = hThread;

    *result = pt;
    return (Error){.etype = ERR_OK, .message = NULL};
}

void playbackResume(PlaybackThread *thread) {
    IAudioClient *ac = (IAudioClient *)thread->audioClient;
    ac->lpVtbl->Start(ac);
}

void playbackPause(PlaybackThread *thread) {
    IAudioClient *ac = (IAudioClient *)thread->audioClient;
    ac->lpVtbl->Stop(ac);
}

void playbackSetUserdata(PlaybackThread *thread, void *userData) {
    thread->userData = userData;
}

void playbackFree(PlaybackThread *thread) {
    SetEvent((HANDLE)thread->hStopEvent);
    WaitForSingleObject((HANDLE)thread->hThread, INFINITE);

    IAudioClient *ac = (IAudioClient *)thread->audioClient;
    IAudioRenderClient *rc = (IAudioRenderClient *)thread->renderClient;
    ac->lpVtbl->Stop(ac);
    rc->lpVtbl->Release(rc);
    ac->lpVtbl->Release(ac);

    CloseHandle((HANDLE)thread->hThread);
    CloseHandle((HANDLE)thread->hStopEvent);
    CloseHandle((HANDLE)thread->hEvent);

    if (thread->comInitialized) CoUninitialize();
    free(thread);
}
