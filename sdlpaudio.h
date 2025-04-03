#ifndef SDLPAUDIO_H
#define SDLPAUDIO_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_AudioDeviceID device;
    SDL_AudioSpec spec;
    int sample_rate;
    int channels;
    int enabled;
    Uint32 audio_pos;
    float time;
    void (*bytebeat_func)(Uint32, float*, int);
} SDLPAudio;

SDLPAudio* sdlpaudio_init(int sample_rate, int channels, void (*bytebeat_func)(Uint32, float*, int));
void sdlpaudio_free(SDLPAudio* audio);
void sdlpaudio_play(SDLPAudio* audio);
void sdlpaudio_pause(SDLPAudio* audio);
void sdlpaudio_set_bytebeat(SDLPAudio* audio, void (*bytebeat_func)(Uint32, float*, int));

#endif