#include "sdlpaudio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AMPLITUDE 8000


static void audio_callback(void* userdata, Uint8* stream, int len) {
    SDLPAudio* audio = (SDLPAudio*)userdata;
    Sint16* buffer = (Sint16*)stream;
    int num_samples = len / (audio->channels * sizeof(Sint16));

    if (!audio->enabled) {
        memset(buffer, 0, len);
        return;
    }

    for (int i = 0; i < num_samples; i++) {
        float sample = 0.0f;
        audio->bytebeat_func(audio->audio_pos, &sample, audio->sample_rate);
        // Левый и правый каналы (чередуем)
        buffer[2 * i] = (Sint16)(AMPLITUDE * sample);     // Левый
        buffer[2 * i + 1] = (Sint16)(AMPLITUDE * sample); // Правый
        audio->audio_pos++;
        printf("t: %u, sample: %f\n", audio->audio_pos, sample); // Отладка
    }
}

SDLPAudio* sdlpaudio_init(int sample_rate, int channels, void (*bytebeat_func)(Uint32, float*, int)) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) return NULL;
    SDLPAudio* audio = (SDLPAudio*)malloc(sizeof(SDLPAudio));
    if (!audio) return NULL;

    audio->sample_rate = sample_rate;
    audio->channels = channels;
    audio->enabled = 1;
    audio->audio_pos = 0;
    audio->time = 0.0f;
    audio->bytebeat_func = bytebeat_func;

    SDL_AudioSpec spec = {0};
    spec.freq = sample_rate;
    spec.format = AUDIO_S16SYS;
    spec.channels = channels;
    spec.samples = 1024;           // 1024				8192
    spec.callback = audio_callback;
    spec.userdata = audio;

    audio->device = SDL_OpenAudioDevice(NULL, 0, &spec, &audio->spec, 0);
    if (audio->device == 0) {
        free(audio);
        return NULL;
    }
    return audio;
}

void sdlpaudio_free(SDLPAudio* audio) {
    if (audio) {
        SDL_CloseAudioDevice(audio->device);
        free(audio);
    }
}

void sdlpaudio_play(SDLPAudio* audio) {
    if (audio) {
        audio->enabled = 1;
        SDL_PauseAudioDevice(audio->device, 0);
    }
}

void sdlpaudio_pause(SDLPAudio* audio) {
    if (audio) {
        audio->enabled = 0;
        SDL_PauseAudioDevice(audio->device, 1);
    }
}

void sdlpaudio_set_bytebeat(SDLPAudio* audio, void (*bytebeat_func)(Uint32, float*, int)) {
    if (audio) {
        SDL_LockAudioDevice(audio->device);
        audio->bytebeat_func = bytebeat_func;
        audio->audio_pos = 0;
        audio->time = 0.0f;
        SDL_UnlockAudioDevice(audio->device);
    }
}
