#include "audio.h"

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <stdio.h>

typedef struct {
    IAudioClient* client;
    IAudioRenderClient* render;
    WAVEFORMATEX* fmt;
    UINT32 bufferFrameCount;
} AudioState;

static AudioState g_audio;

int audio_init() {
    HRESULT hr;
    IMMDeviceEnumerator* enumr = NULL;
    IMMDevice* device = NULL;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                          &IID_IMMDeviceEnumerator, (void**)&enumr);
    if (FAILED(hr)) return -1;

    hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumr, eRender, eConsole, &device);
    IMMDeviceEnumerator_Release(enumr);
    if (FAILED(hr)) return -2;

    hr = IMMDevice_Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&g_audio.client);
    IMMDevice_Release(device);
    if (FAILED(hr)) return -3;

    hr = IAudioClient_GetMixFormat(g_audio.client, &g_audio.fmt);
    if (FAILED(hr)) return -4;
    
    hr = IAudioClient_Initialize(g_audio.client, AUDCLNT_SHAREMODE_SHARED,
                                 0, 0, 0, g_audio.fmt, NULL);
    if (FAILED(hr)) return -5;

    hr = IAudioClient_GetBufferSize(g_audio.client, &g_audio.bufferFrameCount);
    if (FAILED(hr)) return -6;

    hr = IAudioClient_GetService(g_audio.client, &IID_IAudioRenderClient, (void**)&g_audio.render);
    if (FAILED(hr)) return -7;

    hr = IAudioClient_Start(g_audio.client);
    if (FAILED(hr)) return -8;

    return 0;
}

void audio_push(const BYTE* data, UINT32 frames) {
    UINT32 padding = 0;
    IAudioClient_GetCurrentPadding(g_audio.client, &padding);
    UINT32 avail = g_audio.bufferFrameCount - padding;

    if (frames > avail) frames = avail;

    BYTE* buf = NULL;
    IAudioRenderClient_GetBuffer(g_audio.render, frames, &buf);
    memcpy(buf, data, frames * g_audio.fmt->nBlockAlign);
    IAudioRenderClient_ReleaseBuffer(g_audio.render, frames, 0);
}

void audio_shutdown() {
    IAudioClient_Stop(g_audio.client);
    IAudioRenderClient_Release(g_audio.render);
    CoTaskMemFree(g_audio.fmt);
    IAudioClient_Release(g_audio.client);
    CoUninitialize();
}