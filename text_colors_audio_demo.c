/*
MIT License

Copyright (c) Ivan Svarkovsky - 2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*

    gcc -std=c99 -o text_colors_audio_demo text_colors_audio_demo.c sdlgfx.c sdlfont.c -lSDL2 -lm -fopenmp

    gcc -std=c99 -o text_colors_audio_demo text_colors_audio_demo.c sdlgfx.c sdlfont.c -lSDL2 -lm -O3 -fopenmp \
    -march=native -mtune=native -msse3 -mssse3 \
    -fno-exceptions -fomit-frame-pointer -flto=auto -fvisibility=hidden -mfpmath=sse -ffast-math -pipe \
    -s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wl,--strip-all -DNDEBUG


    -fopenmp - многопоточность в сцене - COLOR: CYCLING PALETTE (Belousov-Zhabotinsky)






gcc -std=c99 -o text_colors_audio_demo text_colors_audio_demo.c sdlgfx.c sdlfont.c -lSDL2 -lm -Ofast -fopenmp \
-march=native -mtune=native \
-fno-exceptions -fomit-frame-pointer -flto=$(nproc) -fvisibility=hidden -mfpmath=sse -pipe \
-s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wl,--strip-all -DNDEBUG \
-funroll-loops -ftree-vectorize -ftree-vectorize -falign-functions=16 -falign-loops=16 -fipa-pta -fgraphite-identity \
-floop-nest-optimize -fno-stack-protector 


Замени -O3 -ffast-math на -Ofast — это включит все подфлаги -ffast-math и добавит дополнительные оптимизации.
Добавь -funroll-loops для разворачивания циклов.
Если процессор поддерживает AVX/AVX2, убедись, что -march=native использует эти инструкции, или явно добавь -mavx / -mavx2.
Попробуй -floop-interchange для оптимизации вложенных циклов.


В три этапа
1.
gcc -std=c99 -o text_colors_audio_demo text_colors_audio_demo.c sdlgfx.c sdlfont.c -lSDL2 -lm -Ofast -fopenmp \
-march=native -mtune=native -msse3 -mssse3 \
-fno-exceptions -fomit-frame-pointer -flto=$(nproc) -fvisibility=hidden -mfpmath=sse -pipe \
-s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wl,--strip-all -DNDEBUG \
-funroll-loops -floop-interchange -fprofile-generate -fprofile-correction

2.
./text_colors_audio_demo

3.
gcc -std=c99 -o text_colors_audio_demo text_colors_audio_demo.c sdlgfx.c sdlfont.c -lSDL2 -lm -Ofast -fopenmp \
-march=native -mtune=native -msse3 -mssse3 \
-fno-exceptions -fomit-frame-pointer -flto=$(nproc) -fvisibility=hidden -mfpmath=sse -pipe \
-s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wl,--strip-all -DNDEBUG \
-funroll-loops -floop-interchange -fprofile-use -fprofile-correction


*/


#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "sdlgfx.h"
#include "sdlfont.h"
#include <time.h>

#include <immintrin.h> // Для SSE, SSE2, SSE3, SSSE3 и других инструкций
#include <xmmintrin.h> // Для базовых SSE-инструкций (опционально)
/*
    xmmintrin.h (для SSE, SSE2, SSE3)
    pmmintrin.h (для SSE3, SSSE3)
    emmintrin.h (для SSE2, Supplemental Streaming SIMD Extensions 2 - историческое название, сейчас функциональность SSE2 в основном в xmmintrin.h)
    tmmintrin.h (для SSSE3 - Supplemental Streaming SIMD Extensions 3)
    smmintrin.h (для SSE4.1 - Streaming SIMD Extensions 4.1)
    nmmintrin.h (для SSE4.2 - Streaming SIMD Extensions 4.2)
    avxintrin.h (для AVX - Advanced Vector Extensions)
    avx2intrin.h (для AVX2 - Advanced Vector Extensions 2)
    avx512f.h, avx512vl.h, ... (для AVX-512)
    и другие.
*/

#ifndef M_PI
    #define M_PI acos(-1.0)
#endif

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define SAMPLE_RATE 44100
#define AMPLITUDE 8000
#define CHANNEL_COUNT 2
#define NUM_SOUND_TECHNIQUES 8
#define NUM_COLOR_TECHNIQUES 5
#define MELODY_LENGTH 12

static int audio_pos = 0;
static float sound_time = 0.0;
static int sound_enabled = 1;
static int current_sound_technique = 0;
static float technique_parameters[NUM_SOUND_TECHNIQUES];

int current_color_technique = 0;

char color_technique_names[NUM_COLOR_TECHNIQUES][50] = {
    "COLOR: RAINBOW (HSV)", "COLOR: CYCLING PALETTE",
    "COLOR: RGB OSCILLATIONS", "COLOR: GRADIENT PALETTE",
    "COLOR: RANDOM COLORS" };
char sound_technique_names[NUM_SOUND_TECHNIQUES][50] = {
    "SOUND: TRIANGLE WAVE", "SOUND: PULSE WAVE", "SOUND: NOISE",
    "SOUND: SUB-OSCILLATOR", "SOUND: WAVETABLE SYNTH",
    "SOUND: FORMANT WAVE (SIMP)", "SOUND: MATH FUNCTION (FM)",
    "SOUND: BYTEBEAT MELODY" };

#define WAVETABLE_SIZE 256
float wavetable[WAVETABLE_SIZE];
void init_wavetable() {
    for (int i = 0; i < WAVETABLE_SIZE; i++) {
        wavetable[i] = sin(2 * M_PI * i / WAVETABLE_SIZE); } }

typedef struct {
    float frequency;
    float duration; } Note;
/*
    Note melody[MELODY_LENGTH] = {
    {261.63, 0.2}, {293.66, 0.2}, {329.63, 0.2}, {349.23, 0.4},
    {392.00, 0.2}, {440.00, 0.2}, {493.88, 0.2}, {523.25, 0.4},
    {392.00, 0.2}, {349.23, 0.2}, {329.63, 0.4}, {261.63, 0.4}
    };*/
Note melody[MELODY_LENGTH] = {
    {329.63, 0.4 }, // E4
    {329.63, 0.4 }, // E4
    {349.23, 0.4 }, // F4
    {392.00, 0.4 }, // G4
    {392.00, 0.4 }, // G4
    {349.23, 0.4 }, // F4
    {329.63, 0.4 }, // E4
    {293.66, 0.4 }, // D4
    {261.63, 0.4 }, // C4
    {261.63, 0.4 }, // C4
    {293.66, 0.4 }, // D4
    {329.63, 0.8 } // E4 (длинная)
};


int num_notes = sizeof(melody) / sizeof(melody[0]);
static int current_note = 0;
static float note_time = 0.0;

void get_rainbow_color(float t, int *r, int *g, int *b) {
    *r = (int)(sin(t) * 127 + 128);
    *g = (int)(sin(t + 2 * M_PI / 3) * 127 + 128);
    *b = (int)(sin(t + 4 * M_PI / 3) * 127 + 128); }

float generate_waveform(float time, int waveform_type, float frequency) {
    float sample = 0.0;
    Uint32 t = (Uint32)(time * SAMPLE_RATE);
    switch (waveform_type) {
        case 0: // Triangle wave
            sample = 2.0f * fabs(fmod(frequency * time, 1.0f) - 0.5f) - 1.0f;
            break;
        case 1: // Pulse wave
            sample = (fmod(frequency * time, 1.0f) < technique_parameters[waveform_type]) ? 1.0f : -1.0f;
            break;
        case 2: // Noise
            sample = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
            break;
        case 3: // Sub-oscillator
            sample = sin(2 * M_PI * frequency * time) + 0.5 * sin(2 * M_PI * (frequency / 2.0) * time);
            break;
        case 4: { // Wavetable synth
            float index = fmod(frequency * time * WAVETABLE_SIZE, WAVETABLE_SIZE);
            int index_int = (int)index;
            float fraction = index - index_int;
            float sample1 = wavetable[index_int];
            float sample2 = wavetable[(index_int + 1) % WAVETABLE_SIZE];
            sample = sample1 + (sample2 - sample1) * fraction;
            break; }
        case 5: // Formant wave (simp)
            sample = sin(2 * M_PI * frequency * time + 2 * sin(2 * M_PI * 5 * time));
            break;
        case 6: // Math function (FM)
            sample = sin(2 * M_PI * frequency * time + technique_parameters[waveform_type] * sin(2 * M_PI * 5 * time));
            break;
        case 7: { // Bytebeat melody
            Uint32 result = t * (3 + (1 ^ (t >> 10) & 5)) * (5 + (3 & (t >> 14))) >> (t >> 8 & 3);
            sample = (float)(result & 0xFF) / 128.0f - 1.0f;
            break; } }
    return sample; }

#define ECHO_BUFFER_SIZE (SAMPLE_RATE * 2)
static float echo_buffer[ECHO_BUFFER_SIZE] = {0};
static int echo_pos = 0;

float apply_vibrato(float sample, float time, float frequency, float vibrato_depth, float vibrato_rate) {
    float vibrato = sin(2 * M_PI * vibrato_rate * time) * vibrato_depth;
    return sin(2 * M_PI * (frequency + vibrato) * time); }

float apply_tremolo(float sample, float time, float tremolo_depth, float tremolo_rate) {
    float tremolo = (1.0f - tremolo_depth) + tremolo_depth * (sin(2 * M_PI * tremolo_rate * time) + 1.0f) / 2.0f;
    return sample * tremolo; }

float apply_adsr(float sample, float note_time, float duration, float attack_time, float decay_time, float sustain_level, float release_time) {
    float envelope = 0.0f;
    if (note_time < attack_time) {
        envelope = note_time / attack_time; }
    else if (note_time < attack_time + decay_time) {
        envelope = 1.0f - (1.0f - sustain_level) * (note_time - attack_time) / decay_time; }
    else if (note_time < duration - release_time) {
        envelope = sustain_level; }
    else if (note_time < duration) {
        envelope = sustain_level * (duration - note_time) / release_time; }
    return sample * envelope; }

float apply_harmony(float sample, float time, float frequency, float harmony_ratio) {
    float harmony = sin(2 * M_PI * frequency * harmony_ratio * time);
    return (sample + harmony * 0.5f) / 1.5f; }

float apply_echo(float sample, float time, float echo_delay, float echo_feedback) {
    int delay_samples = (int)(echo_delay * SAMPLE_RATE);
    int read_pos = (echo_pos - delay_samples + ECHO_BUFFER_SIZE) % ECHO_BUFFER_SIZE;
    float echo_sample = echo_buffer[read_pos];
    echo_buffer[echo_pos] = sample + echo_sample * echo_feedback;
    echo_pos = (echo_pos + 1) % ECHO_BUFFER_SIZE;
    return sample + echo_sample * 0.5f; }

void audio_callback(void *userdata, Uint8 *stream, int len) {
    Sint16 *buffer = (Sint16 *)stream;
    int num_samples = len / (CHANNEL_COUNT * sizeof(Sint16));
    if (!sound_enabled) {
        memset(buffer, 0, len);
        return; }
    float vibrato_depth = 5.0f, vibrato_rate = 6.0f;
    float tremolo_depth = 0.3f, tremolo_rate = 4.0f;
    float attack_time = 0.1f, decay_time = 0.2f, sustain_level = 0.7f, release_time = 0.3f;
    float harmony_ratio = 1.5f;
    float echo_delay = 0.25f, echo_feedback = 0.5f;
    for (int frame = 0; frame < num_samples / CHANNEL_COUNT; frame++) {
        float t = (float)audio_pos / SAMPLE_RATE;
        float frequency = melody[current_note].frequency;
        float duration = melody[current_note].duration;
        float sample = generate_waveform(t, current_sound_technique, frequency);
        if (current_sound_technique != 7) {
            sample = apply_vibrato(sample, t, frequency, vibrato_depth, vibrato_rate);
            sample = apply_tremolo(sample, t, tremolo_depth, tremolo_rate);
            sample = apply_adsr(sample, note_time, duration, attack_time, decay_time, sustain_level, release_time);
            float harmony = apply_harmony(sample, t, frequency, harmony_ratio);
            sample = apply_echo(sample, t, echo_delay, echo_feedback);
            buffer[2 * frame] = (Sint16)(AMPLITUDE * sample);
            buffer[2 * frame + 1] = (Sint16)(AMPLITUDE * harmony); }
        else {
            buffer[2 * frame] = (Sint16)(AMPLITUDE * sample);
            buffer[2 * frame + 1] = (Sint16)(AMPLITUDE * sample); }
        audio_pos++;
        sound_time += 1.0 / SAMPLE_RATE;
        note_time += 1.0 / SAMPLE_RATE;
        if (note_time >= duration) {
            note_time = 0.0;
            current_note = (current_note + 1) % num_notes; } } }

SDL_Color color_cycling_palette[16];
void init_color_cycling_palette() {
    color_cycling_palette[0]  = (SDL_Color) {
        220, 20,  20,  255 }; // Ярко-красный
    color_cycling_palette[1]  = (SDL_Color) {
        200, 50,  30,  255 }; // Насыщенный красный
    color_cycling_palette[2]  = (SDL_Color) {
        255, 100, 0,   255 }; // Оранжево-красный
    color_cycling_palette[3]  = (SDL_Color) {
        255, 150, 50,  255 }; // Ярко-оранжевый
    color_cycling_palette[4]  = (SDL_Color) {
        255, 200, 100, 255 }; // Светло-оранжевый
    color_cycling_palette[5]  = (SDL_Color) {
        255, 240, 180, 255 }; // Бледно-желтый (почти бесцветный)
    color_cycling_palette[6]  = (SDL_Color) {
        255, 255, 240, 255 }; // Очень светло-желтый (почти белый)
    color_cycling_palette[7]  = (SDL_Color) {
        200, 240, 255, 255 }; // Светло-голубой
    color_cycling_palette[8]  = (SDL_Color) {
        100, 180, 255, 255 }; // Голубой
    color_cycling_palette[9]  = (SDL_Color) {
        50,  120, 255, 255 }; // Ярко-синий
    color_cycling_palette[10] = (SDL_Color) {
        30,  80,  200, 255 }; // Насыщенный синий
    color_cycling_palette[11] = (SDL_Color) {
        20,  50,  150, 255 }; // Темно-синий
    color_cycling_palette[12] = (SDL_Color) {
        50,  50,  100, 255 }; // Очень темно-синий (почти черный)
    color_cycling_palette[13] = (SDL_Color) {
        80,  30,  80,  255 }; // Темно-фиолетовый
    color_cycling_palette[14] = (SDL_Color) {
        150, 30,  50,  255 }; // Красно-фиолетовый
    color_cycling_palette[15] = (SDL_Color) {
        220, 20,  20,  255 }; // Ярко-красный (возврат к началу)
}

SDL_Color get_color_cycling_color(float time) {
    int palette_index = (int)(time * 10) % 16;
    return color_cycling_palette[palette_index]; }

SDL_Color get_rgb_oscillations_color(float time) {
    int r = (int)(sin(time * 2) * 127 + 128);
    int g = (int)(cos(time * 3) * 127 + 128);
    int b = (int)(sin(time * 5 + M_PI/2) * 127 + 128);
    return (SDL_Color) {
        r, g, b, 255 }; }

SDL_Color gradient_palette[16];
void init_gradient_palette() {
    for (int i = 0; i < 16; i++) {
        float t = (float)i / 15.0f;
        int r = 0;
        int g = (int)(t * 255);
        int b = 255 - (int)(t * 255);
        gradient_palette[i] = (SDL_Color) {
            r, g, b, 255 }; } }

SDL_Color get_gradient_palette_color(float x, int width) {
    int palette_index = (int)((float)x / width * 15.0f) % 16;
    return gradient_palette[palette_index]; }

SDL_Color random_color_target = {0, 0, 0, 255 };
SDL_Color current_random_color = {0, 0, 0, 255 };
float random_color_interpolation_factor = 0.02f;
SDL_Color get_smooth_random_color() {
    float diff_r = abs(current_random_color.r - random_color_target.r);
    float diff_g = abs(current_random_color.g - random_color_target.g);
    float diff_b = abs(current_random_color.b - random_color_target.b);
    if (diff_r < 2 && diff_g < 2 && diff_b < 2) {
        random_color_target = (SDL_Color) {
            rand() % 256, rand() % 256, rand() % 256, 255 }; }
    current_random_color.r += (random_color_target.r - current_random_color.r) * random_color_interpolation_factor;
    current_random_color.g += (random_color_target.g - current_random_color.g) * random_color_interpolation_factor;
    current_random_color.b += (random_color_target.b - current_random_color.b) * random_color_interpolation_factor;
    return current_random_color; }

void draw_background(int width, int height, float time, int color_technique) {
    switch (color_technique) {
        case 0:
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    float dist_center_norm = sqrt(pow(x - width/2.0f, 2) + pow(y - height/2.0f, 2)) / (height/2.0f);
                    if (dist_center_norm > 1.0f) {
                        dist_center_norm = 1.0f; }
                    int r, g, b;
                    get_rainbow_color(time * 0.5f + dist_center_norm * 5.0f, &r, &g, &b);
                    sdlgfx_color(r, g, b);
                    sdlgfx_pixel(x, y); } }
            break;

/*
  case 1: // CYCLING PALETTE
            for (int y = 0; y < height; y+= 4) {
                for (int x = 0; x < width; x+= 4) {
                    float center_x = width / 2.0f;
                    float center_y = height / 2.0f;
                    float dx = x - center_x;
                    float dy = y - center_y;
                    float angle = atan2f(dy, dx); // Угол от центра до точки (от -PI до PI)
                    float distance = sqrtf(dx * dx + dy * dy);

                    // Комбинация угла и расстояния для "завихрения"
                    float swirl = sinf(angle * 3.0f + distance * 0.01f + time); // 3.0f - количество "рукавов" завихрения

                    SDL_Color color = get_color_cycling_color(swirl * 0.5f + 0.5f);

                    sdlgfx_color(color.r, color.g, color.b);
                    sdlgfx_rectangle(x, y, x + 1, y + 1);
                }
            }
            break;
*/

        case 1: { // COLOR: CYCLING PALETTE (Belousov-Zhabotinsky with Full Update - [DISTANCE TYPE], Static Schedule, LOGGING NORMALIZATION)
            // Статическая текстура для повторного использования
            static SDL_Texture* temp_tex = NULL;
            static int tex_width = 0, tex_height = 0;
            static int frame_count = 0; // Статический счетчик кадров
            // 1. Предварительные вычисления
            int num_centers = 3;
            int centers_x[3] = {
                (int)((float)width * 0.25f),
                (int)((float)width * 0.75f),
                (int)((float)width * 0.5f) };
            int centers_y[3] = {
                (int)((float)height * 0.25f),
                (int)((float)height * 0.75f),
                (int)((float)height * 0.5f) };
            float wave_frequency = 0.00005f;  // 0.00005f;        0.025f  0.05f
            float wave_speed = 0.04f;  // 0.04f;
            const int STEP = 1; // 2   можем, но не используем

#define SIN_LUT_SIZE 512 // Степень двойки (2^9)

            static float sin_lut[SIN_LUT_SIZE];
            static int lut_initialized = 0;
            // 0. **Реализация _mm_abs_ps для SSE2**
            inline __m128 _mm_abs_ps_sse2(__m128 x) {
                __m128 sign_mask = _mm_set1_ps(-0.0f);
                return _mm_andnot_ps(sign_mask, x); }
            // 2. Создание или проверка текстуры
            if (!temp_tex || tex_width != width || tex_height != height) {
                if (temp_tex) {
                    SDL_DestroyTexture(temp_tex); }
                temp_tex = SDL_CreateTexture(sdlgfx_renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             width,
                                             height);
                if (!temp_tex) {
                    fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
                    break; }
                tex_width = width;
                tex_height = height; }
            // 3. Выделяем формат пикселей
            SDL_PixelFormat *pixel_format = SDL_AllocFormat(SDL_GetWindowPixelFormat(sdlgfx_window));
            if (!pixel_format) {
                fprintf(stderr, "SDL_AllocFormat Error: %s\n", SDL_GetError());
                break; }
            // 4. Прямой доступ к пиксельному буферу
            Uint32* pixels;
            int pitch;
            if (SDL_LockTexture(temp_tex, NULL, (void**)&pixels, &pitch) != 0) {
                fprintf(stderr, "SDL_LockTexture Error: %s\n", SDL_GetError());
                SDL_FreeFormat(pixel_format);
                break; }
            pitch /= sizeof(Uint32);
            // Инициализация LUT для синусоиды
            if (!lut_initialized) {
                for (int i = 0; i < SIN_LUT_SIZE; ++i) {
                    float angle = (float)i / SIN_LUT_SIZE * 2.0f * M_PI;
                    sin_lut[i] = sinf(angle); }
                lut_initialized = 1; }
            // **5. Инициализация min_phase и max_phase для кадра**
            float min_phase = 1e30f;
            float max_phase = -1e30f;
            // Основной цикл с векторизацией (SSE) и многопоточностью для всего экрана - [DISTANCE TYPE], schedule(static), LOGGING NORMALIZATION
            #pragma omp parallel for collapse(2) schedule(static)
            for (int y = 0; y < height; y += STEP) {
                for (int x = 0; x < width; x += 4 * STEP) {
                    if (x + 4 * STEP > width) {
                        continue; }
                    __m128 x_vec = _mm_set_ps((float)(x + 3 * STEP), (float)(x + 2 * STEP), (float)(x + STEP), (float)x);
                    __m128 y_vec = _mm_set1_ps((float)y);
                    __m128 total_phase_vec = _mm_setzero_ps();
                    for (int i = 0; i < num_centers; i++) {
                        __m128 center_x_vec = _mm_set1_ps((float)centers_x[i]);
                        __m128 center_y_vec = _mm_set1_ps((float)centers_y[i]);
                        __m128 dx_vec = _mm_sub_ps(x_vec, center_x_vec);
                        __m128 dy_vec = _mm_sub_ps(y_vec, center_y_vec);
                        __m128 distance_squared_vec; // Объявляем здесь, чтобы переопределить ниже

                        // *** ВЫБЕРИТЕ ОДИН ИЗ ВАРИАНТОВ РАССТОЯНИЯ ДЛЯ ТЕСТИРОВАНИЯ ***
                        // *** ВАРИАНТ 1: МАНХЭТТЕНСКОЕ РАССТОЯНИЕ (ИСХОДНЫЙ КОД - ЗАКОММЕНТИРОВАН ДЛЯ НАЧАЛА) ***
                        // distance_squared_vec = _mm_add_ps(
                        //     _mm_abs_ps_sse2(dx_vec),
                        //     _mm_abs_ps_sse2(dy_vec)
                        // );
                        // *** ВАРИАНТ 2: ТОЛЬКО ГОРИЗОНТАЛЬНОЕ РАССТОЯНИЕ (ОЧЕНЬ ПРОСТО) ***
                        // distance_squared_vec = _mm_abs_ps_sse2(dx_vec);
                        // *** ВАРИАНТ 3: ТОЛЬКО ВЕРТИКАЛЬНОЕ РАССТОЯНИЕ (ОЧЕНЬ ПРОСТО) ***
                        // distance_squared_vec = _mm_abs_ps_sse2(dy_vec);
                        // *** ВАРИАНТ 4: СУММА АБСОЛЮТНЫХ ЗНАЧЕНИЙ КООРДИНАТ БЕЗ ВЫЧИТАНИЯ ЦЕНТРА (ЕЩЕ ПРОЩЕ) ***
                        // distance_squared_vec = _mm_add_ps(_mm_abs_ps_sse2(x_vec), _mm_abs_ps_sse2(y_vec));
                        // *** ВАРИАНТ 5: ПРОСТО СУММА КООРДИНАТ БЕЗ АБСОЛЮТНЫХ ЗНАЧЕНИЙ И ВЫЧИТАНИЯ ЦЕНТРА (САМОЕ ПРОСТОЕ, ГРАДИЕНТ) ***
                        // distance_squared_vec = _mm_add_ps(x_vec, y_vec);
                        // *** ВАРИАНТ 6: РАЗНОСТЬ КООРДИНАТ (ДИАГОНАЛЬНЫЙ ГРАДИЕНТ) ***
                        // distance_squared_vec = _mm_sub_ps(x_vec, y_vec);
                        // *** ВАРИАНТ 7: ИСХОДНОЕ ЕВКЛИДОВО РАССТОЯНИЕ (ДЛЯ СРАВНЕНИЯ) ***
                         distance_squared_vec = _mm_add_ps(
                             _mm_mul_ps(dx_vec, dx_vec),
                             _mm_mul_ps(dy_vec, dy_vec)
                         );
                        __m128 phase_input_vec = _mm_add_ps(
                                                     _mm_mul_ps(distance_squared_vec, _mm_set1_ps(wave_frequency)),
                                                     _mm_set1_ps(-time * wave_speed + i * 2.0f)
                                                 );
                        // Упрощённая синусоида через LUT без нормализации к [0, 2π]
                        __m128 lut_scale = _mm_set1_ps((float)SIN_LUT_SIZE / (2.0f * M_PI));
                        __m128 lut_index_vec = _mm_mul_ps(phase_input_vec, lut_scale);
                        float phase_inputs[4];
                        _mm_store_ps(phase_inputs, lut_index_vec);
                        for (int k = 0; k < 4; k++) {
                            int idx = (int)phase_inputs[k] & (SIN_LUT_SIZE - 1); // Побитовое И вместо %
                            if (idx < 0) {
                                idx += SIN_LUT_SIZE; } // Обрабатываем отрицательные значения
                            ((float*)&total_phase_vec)[k] += sin_lut[idx]; } }
                    // **6. Обновление min_phase и max_phase (ДО нормализации)**
                    float current_phases[4];
                    _mm_store_ps(current_phases, total_phase_vec);
                    for (int k = 0; k < 4; k++) {
                        if (current_phases[k] < min_phase) {
                            min_phase = current_phases[k]; }
                        if (current_phases[k] > max_phase) {
                            max_phase = current_phases[k]; } }
                    // Нормализация к [0, 1] - **ОСТАВЛЯЕМ ИСХОДНУЮ НОРМАЛИЗАЦИЮ ДЛЯ ВИДИМОГО РЕЗУЛЬТАТА**
                    __m128 normalized_phase_vec = _mm_add_ps(
                                                      _mm_mul_ps(total_phase_vec, _mm_set1_ps(0.5f / num_centers)), // Исходная нормализация   0.5f
                                                      _mm_set1_ps(0.5f)
                                                  );
                    normalized_phase_vec = _mm_max_ps(normalized_phase_vec, _mm_setzero_ps());
                    normalized_phase_vec = _mm_min_ps(normalized_phase_vec, _mm_set1_ps(1.0f));
                    float normalized_phases[4];
                    _mm_store_ps(normalized_phases, normalized_phase_vec);
                    // Заполнение пикселей с исходной палитрой
                    for (int k = 0; k < 4; k++) {
                        SDL_Color color = get_color_cycling_color(normalized_phases[k]);
                        Uint32 pixel_color = (255 << 24) | (color.r << 16) | (color.g << 8) | color.b;
                        int current_x = x + k * STEP;
                        const int max_x = (current_x + STEP < width) ? current_x + STEP : width;
                        const int max_y = (y + STEP < height) ? y + STEP : height;
                        for (int yy = y; yy < max_y; yy++) {
                            for (int xx = current_x; xx < max_x; xx++) {
                                pixels[yy * pitch + xx] = pixel_color; } } } } }
            /*
                // **7. Вывод лога в консоль ПОСЛЕ цикла отрисовки**
                frame_count++;
                float phase_range = max_phase - min_phase;
                float suggested_scale_factor = 0.0f;
                if (phase_range > 0.0f) {
                    suggested_scale_factor = 1.0f / phase_range;
                }

                printf("Frame: %d, Distance Type: [ВЕРТИКАЛЬНОЕ РАССТОЯНИЕ], Min Phase: %.2f, Max Phase: %.2f, Range: %.2f, Suggested Scale Factor: %.6f\n",
                       // Изменил Distance Type в логе
                       frame_count, min_phase, max_phase, phase_range, suggested_scale_factor);

            */
            // Пакетная отрисовка
            SDL_UnlockTexture(temp_tex);
            SDL_FreeFormat(pixel_format);
            SDL_RenderCopy(sdlgfx_renderer, temp_tex, NULL, NULL); }
        break;

        case 2:
            for (int y = 0; y < height; y += 6) {
                for (int x = 0; x < width; x += 6) {
                    SDL_Color color = get_rgb_oscillations_color(time + (x + y) * 0.005f);
                    sdlgfx_color(color.r, color.g, color.b);
                    sdlgfx_fill_rectangle(x, y, x + 6, y + 6); } }
            break;
        case 3:
            for (int i = 0; i < 150; i++) {
                float angle = time * 0.3f + i * 0.04f;
                float length = 200 + sin(time * 1.5f + i * 0.1f) * 50;
                float start_x = width / 2.0f;
                float start_y = height / 2.0f;
                float end_x = start_x + cos(angle) * length;
                float end_y = start_y + sin(angle) * length;
                SDL_Color color = get_gradient_palette_color(i * 5, width * 2);
                sdlgfx_color(color.r, color.g, color.b);
                sdlgfx_line(start_x, start_y, end_x, end_y); }
            break;
        case 4:
            SDL_Color bg_color = get_smooth_random_color();
            sdlgfx_color(bg_color.r, bg_color.g, bg_color.b);
            sdlgfx_fill_rectangle(0, 0, width, height);
            break; } }



void draw_massive_scene(int width, int height, float time, int color_technique) {
    // 1. Отображение текстуры фона:
    //    Эта строка отрисовывает на экране текстуру, которая была предварительно создана
    //    функцией `draw_background` и сохранена в `sdlgfx_texture`.
    //    Фон обновляется только тогда, когда это необходимо (например, при смене цветовой техники
    //    или если фон должен динамически меняться каждый кадр).
    SDL_RenderCopy(sdlgfx_renderer, sdlgfx_texture, NULL, NULL);
    // 2. Условное рисование сетки:
    //    Этот блок кода рисует сетку поверх фона, но только если текущая цветовая техника
    //    НЕ является "COLOR: CYCLING PALETTE" (индекс 1).
    //    Это условие было добавлено, чтобы убрать сетку для техники CYCLING PALETTE,
    //    так как она визуально не очень сочеталась с этой техникой.
    if (color_technique != 1) {
        // 2.1. Определение параметров сетки:
        //      Эти #define задают параметры внешнего вида сетки:
#define GRID_COLOR_R 30       // Красный компонент цвета сетки
#define GRID_COLOR_G 30       // Зеленый компонент цвета сетки
#define GRID_COLOR_B 30       // Синий компонент цвета сетки
#define GRID_SPACING 5        // Желаемое количество ячеек сетки по короткой стороне экрана
#define GRID_LINE_THICKNESS 1 // Толщина линий сетки
        // 2.2. Установка цвета сетки:
        //      Используем заданные RGB компоненты для установки цвета рисования в SDLGFX.
        sdlgfx_color(GRID_COLOR_R, GRID_COLOR_G, GRID_COLOR_B);
        // 2.3. Расчет размеров ячеек сетки:
        //      Определяем, какая сторона экрана короче, какая длиннее.
        float shorter_side = (width < height) ? width : height;
        float longer_side = (width < height) ? height : width;
        int target_cells_shorter_side = GRID_SPACING; // Целевое количество ячеек по короткой стороне
        int num_cells_shorter_side = target_cells_shorter_side; // Фактическое количество ячеек по короткой стороне
        //      Рассчитываем количество ячеек по длинной стороне, чтобы сетка была пропорциональной.
        int num_cells_longer_side = (int)round((float)target_cells_shorter_side * (longer_side / shorter_side));
        //      Рассчитываем шаг (размер ячейки) по X и Y, чтобы заполнить весь экран сеткой.
        float step_x, step_y;
        if (width < height) {
            step_x = (float)width / num_cells_shorter_side;
            step_y = (float)height / num_cells_longer_side; }
        else {
            step_x = (float)width / num_cells_longer_side;
            step_y = (float)height / num_cells_shorter_side; }
        // 2.4. Рисование горизонтальных линий сетки:
        //      Цикл по Y координате с шагом `step_y`, рисуем горизонтальные линии.
        //      `thickness_offset` позволяет рисовать линии толщиной в несколько пикселей.
        for (int thickness_offset = 0; thickness_offset < GRID_LINE_THICKNESS; thickness_offset++) {
            for (int y = 0; y < height; y += step_y) {
                sdlgfx_line(0, y + thickness_offset, width, y + thickness_offset); } }
        // 2.5. Рисование вертикальных линий сетки:
        //      Аналогично горизонтальным, цикл по X координате с шагом `step_x`, рисуем вертикальные линии.
        //      `thickness_offset` также используется для толщины линий.
        for (int thickness_offset = 0; thickness_offset < GRID_LINE_THICKNESS; thickness_offset++) {
            for (int x = 0; x < width; x += step_x) {
                sdlgfx_line(x + thickness_offset, 0, x + thickness_offset, height); } } } // Конец условного рисования сетки (if (color_technique != 1))
    // 3. Условное рисование кругов (для техники RAINBOW (HSV)):
    //    Если выбрана техника "COLOR: RAINBOW (HSV)" (индекс 0), рисуем анимированные круги.
    if (color_technique == 0) {
        for (int i = 0; i < 30; i++) {
            // 3.1. Расчет радиуса круга:
            //      Радиус круга меняется со временем и зависит от индекса `i`, создавая эффект пульсации.
            float circle_radius = 50 + sin(time * 2.0f + i * 0.3f) * 40;
            // 3.2. Расчет цвета круга:
            //      Цвет круга берется из радужной палитры, также меняется со временем и индексом `i`.
            int r, g, b;
            get_rainbow_color(time * 1.0f + i * 0.2f, &r, &g, &b);
            // 3.3. Установка цвета и рисование круга:
            sdlgfx_color(r, g, b);
            sdlgfx_circle(width/2 + cos(i*1.2f)*100, height/2 + sin(i*1.2f)*100, abs((int)circle_radius)); } }
    // 4. Условное рисование кругов (для техники RANDOM COLORS):
    //    Если выбрана техника "COLOR: RANDOM COLORS" (индекс 4), рисуем случайные круги.
    else if (color_technique == 4) {
        // 4.1. Установка белого цвета для кругов:
        sdlgfx_color(255, 255, 255);
        for (int i = 0; i < 50; i++) {
            // 4.2. Случайная позиция круга:
            //      Круги располагаются случайным образом в пределах экрана (с отступом 10% от краев).
            float circle_x = width * (0.1 + ((float)rand() / RAND_MAX) * 0.8);
            float circle_y = height * (0.1 + ((float)rand() / RAND_MAX) * 0.8);
            // 4.3. Радиус круга:
            //      Радиус круга слегка меняется со временем, создавая эффект "дыхания".
            int radius = 10 + (int)(sin(time * 2.5 + i * 0.4) * 8);
            // 4.4. Рисование круга:
            sdlgfx_circle(circle_x, circle_y, abs(radius)); } }
    // 5. Рисование фона для текста с названием техники и звука:
    //    Рисуем черный прямоугольник в левом верхнем углу для контраста текста.
    sdlgfx_color(0, 0, 0);
    sdlgfx_fill_rectangle(0, 10, 228, 60);
    // 6. Рисование текста с названием цветовой и звуковой техники:
    //    Рисуем белым цветом названия текущих техник в левом верхнем углу.
    sdlgfx_color(255, 255, 255);
    sdlgfx_string(10, 20, color_technique_names[current_color_technique]); // Название цветовой техники
    sdlgfx_string(10, 40, sound_technique_names[current_sound_technique]);   // Название звуковой техники
}

int needs_update_every_frame(int technique) {
    switch (technique) {
        case 1:
        case 2:
        case 3:
        case 4:
            return 1;
        default:
            return 0; } }

unsigned int technique_start_time;
unsigned int technique_duration = 600000;   //  sec
int countdown_seconds = 0;
int show_countdown = 1;

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1; }
    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = CHANNEL_COUNT;
    spec.samples = 1024;
    spec.callback = audio_callback;
    spec.userdata = &current_sound_technique;  // Передаем указатель на current_sound_technique
    SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (device == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        SDL_Quit();
        return 1; }
    sdlgfx_open(WINDOW_WIDTH, WINDOW_HEIGHT, "Sound & Color Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    sdlgfx_clear_color(0, 0, 0);
    SDL_PauseAudioDevice(device, 0);
    init_color_cycling_palette();
    init_gradient_palette();
    init_wavetable();
    technique_parameters[0] = 0.0f;
    technique_parameters[1] = 0.25f;
    technique_parameters[2] = 0.0f;
    technique_parameters[3] = 0.0f;
    technique_parameters[4] = 0.0f;
    technique_parameters[5] = 0.0f;
    technique_parameters[6] = 5.0f;
    technique_parameters[7] = 0.0f;
    float time = 0.0;
    int running = 1;
    technique_start_time = SDL_GetTicks();
    Uint32 frame_start, frame_time;
    const int target_fps = 60;
    const Uint32 frame_delay = 1000 / target_fps;
    sdlgfx_update_texture(draw_background, WINDOW_WIDTH, WINDOW_HEIGHT, time, current_color_technique, 1);
    while (running) {
        frame_start = SDL_GetTicks();
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_Q)) {
                running = 0; }
            if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_M) {
                sound_enabled = !sound_enabled; }
            if (e.type == SDL_KEYDOWN) {
                show_countdown = 1;
                switch (e.key.keysym.scancode) {
                    case SDL_SCANCODE_S:
                        current_sound_technique = (current_sound_technique + 1) % NUM_SOUND_TECHNIQUES;
                        technique_start_time = SDL_GetTicks();
                        audio_pos = 0;
                        sound_time = 0;
                        current_note = 0;
                        note_time = 0;
                        printf("Switched to sound technique: %s\n", sound_technique_names[current_sound_technique]);
                        break;
                    case SDL_SCANCODE_A:
                        current_sound_technique = (current_sound_technique - 1 + NUM_SOUND_TECHNIQUES) % NUM_SOUND_TECHNIQUES;
                        technique_start_time = SDL_GetTicks();
                        audio_pos = 0;
                        sound_time = 0;
                        current_note = 0;
                        note_time = 0;
                        printf("Switched to sound technique: %s\n", sound_technique_names[current_sound_technique]);
                        break;
                    case SDL_SCANCODE_N:
                        current_color_technique = (current_color_technique + 1) % NUM_COLOR_TECHNIQUES;
                        technique_start_time = SDL_GetTicks();
                        sdlgfx_update_texture(draw_background, WINDOW_WIDTH, WINDOW_HEIGHT, time, current_color_technique, 1);
                        printf("Switched to color technique: %s\n", color_technique_names[current_color_technique]);
                        break;
                    case SDL_SCANCODE_P:
                        current_color_technique = (current_color_technique - 1 + NUM_COLOR_TECHNIQUES) % NUM_COLOR_TECHNIQUES;
                        technique_start_time = SDL_GetTicks();
                        sdlgfx_update_texture(draw_background, WINDOW_WIDTH, WINDOW_HEIGHT, time, current_color_technique, 1);
                        printf("Switched to color technique: %s\n", color_technique_names[current_color_technique]);
                        break; } } }
        time = SDL_GetTicks() / 1000.0f;
        int update_texture = needs_update_every_frame(current_color_technique);
        sdlgfx_update_texture(draw_background, WINDOW_WIDTH, WINDOW_HEIGHT, time, current_color_technique, update_texture);
        sdlgfx_clear();
        draw_massive_scene(WINDOW_WIDTH, WINDOW_HEIGHT, time, current_color_technique);
        sdlgfx_color(255, 255, 255);
        sdlgfx_string(10, WINDOW_HEIGHT - 30, "PRESS 'Q' TO QUIT, 'M' TO MUTE/UNMUTE, 'N'/'P' TO CHANGE COLOR, 'S'/'A' - SOUND");
        if (show_countdown) {
            unsigned int current_time = SDL_GetTicks();
            if (current_time - technique_start_time >= technique_duration) {
                current_color_technique = (current_color_technique + 1) % NUM_COLOR_TECHNIQUES;
                technique_start_time = current_time;
                sdlgfx_update_texture(draw_background, WINDOW_WIDTH, WINDOW_HEIGHT, time, current_color_technique, 1); }
            countdown_seconds = (technique_duration - (current_time - technique_start_time)) / 1000;
            if (countdown_seconds < 0) {
                countdown_seconds = 0; }
            char countdown_text[50];
            sprintf(countdown_text, "Next technique in %d sec...", countdown_seconds);
            sdlgfx_color(0, 0, 0);
            sdlgfx_color(255, 255, 255);
            sdlgfx_string(10, WINDOW_HEIGHT - 15, countdown_text); }
        sdlgfx_flush();
        frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay) {
            SDL_Delay(frame_delay - frame_time); } }
    SDL_PauseAudioDevice(device, 1);
    SDL_CloseAudioDevice(device);
    sdlgfx_close();
    SDL_Quit();
    return 0; }