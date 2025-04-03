// sdlgfx.c
/*

			SDLGFX	v0.1

*/

/**
 * @file sdlgfx.c
 * @brief A simple graphics library based on SDL2.
 * Original gfx library by Ivan Svarkovsky (c) 2019 (Xlib version).
 * SDL2 port and modifications by Ivan Svarkovsky.
 *
 * This library provides basic 2D graphics primitives for drawing points, lines,
 * circles, rectangles, ellipses, arcs and text in an SDL2 window.
 *
 * Compilation (example):
 * gcc -o test_sdlgfx test_sdlgfx.c sdlgfx.c sdlfont.c -lSDL2 -lm
 */

/*
    MIT License

    Copyright (c) Ivan Svarkovsky - 2025	<https://github.com/Svarkovsky>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "sdlgfx.h"
#include "sdlfont.h"
#include <math.h>
#include <emmintrin.h> // SSE2 intrinsics

#ifndef M_PI
    #define M_PI acos(-1.0)
#endif

/* ====================================================================== */
/*                  GLOBAL VARIABLES SECTION                              */
/* ====================================================================== */

SDL_Window *sdlgfx_window = NULL;     //!< SDL window handle.
SDL_Renderer *sdlgfx_renderer = NULL;   //!< SDL renderer handle.
SDL_Texture *sdlgfx_texture = NULL;    //!< SDL texture for rendering.
static int window_width = 0;          //!< Width of the graphics window.
static int window_height = 0;         //!< Height of the graphics window.
static int clear_r = 0;               //!< Red component of the clear color.
static int clear_g = 0;               //!< Green component of the clear color.
static int clear_b = 0;               //!< Blue component of the clear color.
static SDL_Color current_color = {0, 0, 0, 255}; //!< Current drawing color.

/* ====================================================================== */
/*                  EXPORTED LIBRARY FUNCTIONS                           */
/* ====================================================================== */

static int use_streaming_texture = 0; //!< Flag to use streaming texture.
static void *locked_pixels = NULL;     //!< Pointer to locked pixel data.
static int locked_pitch = 0;         //!< Pitch of the locked texture.

/**
 * @brief Enables or disables streaming texture usage.
 * @param enable 1 to enable streaming, 0 to disable.
 */
void sdlgfx_set_streaming_texture(int enable) {
    use_streaming_texture = enable;
    if (sdlgfx_renderer && sdlgfx_texture) {
        // Recreate texture with new access type if renderer and texture already exist
        SDL_DestroyTexture(sdlgfx_texture);
        int access = use_streaming_texture ? SDL_TEXTUREACCESS_STREAMING : SDL_TEXTUREACCESS_TARGET;
        sdlgfx_texture = SDL_CreateTexture(sdlgfx_renderer, SDL_PIXELFORMAT_RGBA8888, access, window_width, window_height);
        if (!sdlgfx_texture) {
            fprintf(stderr, "sdlgfx_set_streaming_texture: SDL_CreateTexture Error: %s\n", SDL_GetError());
        }
    }
}

/**
 * @brief Opens a new SDL graphics window.
 * @param width Window width.
 * @param height Window height.
 * @param title Window title.
 * @param x Initial X position.
 * @param y Initial Y position.
 */
void sdlgfx_open(int width, int height, const char *title, int x, int y) {
    if (sdlgfx_window != NULL) {
        fprintf(stderr, "sdlgfx_open: Warning: graphics window already open.\n");
        return;
    }

    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
            return;
        }
    }

    window_width = width;
    window_height = height;

    sdlgfx_window = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_SHOWN);

    if (sdlgfx_window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    sdlgfx_renderer = SDL_CreateRenderer(sdlgfx_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (sdlgfx_renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdlgfx_window);
        SDL_Quit();
        return;
    }

    int access = use_streaming_texture ? SDL_TEXTUREACCESS_STREAMING : SDL_TEXTUREACCESS_TARGET;
    sdlgfx_texture = SDL_CreateTexture(sdlgfx_renderer, SDL_PIXELFORMAT_RGBA8888, access, width, height);

    if (sdlgfx_texture == NULL) {
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(sdlgfx_renderer);
        SDL_DestroyWindow(sdlgfx_window);
        SDL_Quit();
        return;
    }

    SDL_SetRenderTarget(sdlgfx_renderer, NULL);
    SDL_SetRenderDrawBlendMode(sdlgfx_renderer, SDL_BLENDMODE_BLEND);
    sdlgfx_clear_color(clear_r, clear_g, clear_b);
    sdlgfx_clear();
    fprintf(stdout, "SDL graphics window opened successfully: %dx%d pixels.\n", window_width, window_height);
}

/**
 * @brief Closes the SDL graphics window.
 */
void sdlgfx_close(void) {
    if (sdlgfx_texture) {
        SDL_DestroyTexture(sdlgfx_texture);
        sdlgfx_texture = NULL;
    }

    if (sdlgfx_renderer) {
        SDL_DestroyRenderer(sdlgfx_renderer);
        sdlgfx_renderer = NULL;
    }

    if (sdlgfx_window) {
        SDL_DestroyWindow(sdlgfx_window);
        sdlgfx_window = NULL;
    }

    SDL_Quit();
    fprintf(stdout, "SDL graphics window closed.\n");
}

/**
 * @brief Updates the texture for dynamic drawing.
 * @param draw_func Drawing function callback.
 * @param width Drawing width.
 * @param height Drawing height.
 * @param time Time parameter for drawing function.
 * @param technique Technique parameter for drawing function.
 * @param update Update flag (1 to update, 0 to skip).
 */
void sdlgfx_update_texture(void (*draw_func)(int, int, float, int), int width, int height, float time, int technique, int update) {
    if (update) {
        SDL_SetRenderTarget(sdlgfx_renderer, sdlgfx_texture);
        sdlgfx_clear();
        draw_func(width, height, time, technique);
        SDL_SetRenderTarget(sdlgfx_renderer, NULL);
    }
}

/**
 * @brief Sets the current drawing color.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 */
void sdlgfx_color(int r, int g, int b) {
    current_color.r = r;
    current_color.g = g;
    current_color.b = b;
    current_color.a = 255;
    SDL_SetRenderDrawColor(sdlgfx_renderer, current_color.r, current_color.g, current_color.b, current_color.a);
}

/**
 * @brief Sets the clear color.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 */
void sdlgfx_clear_color(int r, int g, int b) {
    clear_r = r;
    clear_g = g;
    clear_b = b;
}

/**
 * @brief Clears the screen with the clear color.
 */
void sdlgfx_clear(void) {
    SDL_SetRenderDrawColor(sdlgfx_renderer, clear_r, clear_g, clear_b, 255);
    SDL_RenderClear(sdlgfx_renderer);
}

/**
 * @brief Draws a point.
 * @param x X coordinate.
 * @param y Y coordinate.
 */
void sdlgfx_point(int x, int y) {
    SDL_RenderDrawPoint(sdlgfx_renderer, x, y);
}

/**
 * @brief Draws a line.
 * @param x1 Start X coordinate.
 * @param y1 Start Y coordinate.
 * @param x2 End X coordinate.
 * @param y2 End Y coordinate.
 */
void sdlgfx_line(int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(sdlgfx_renderer, x1, y1, x2, y2);
}

/**
 * @brief Draws a rectangle outline.
 * @param x1 Top-left X coordinate.
 * @param y1 Top-left Y coordinate.
 * @param x2 Bottom-right X coordinate.
 * @param y2 Bottom-right Y coordinate.
 */
void sdlgfx_rectangle(int x1, int y1, int x2, int y2) {
    SDL_Rect rect = {x1, y1, x2 - x1, y2 - y1};
    SDL_RenderDrawRect(sdlgfx_renderer, &rect);
}

/**
 * @brief Draws a filled rectangle.
 * @param x1 Top-left X coordinate.
 * @param y1 Top-left Y coordinate.
 * @param x2 Bottom-right X coordinate.
 * @param y2 Bottom-right Y coordinate.
 */
void sdlgfx_fill_rectangle(int x1, int y1, int x2, int y2) {
    SDL_Rect rect = {x1, y1, x2 - x1, y2 - y1};
    SDL_RenderFillRect(sdlgfx_renderer, &rect);
}

/**
 * @brief Draws a circle outline.
 * @param x Center X coordinate.
 * @param y Center Y coordinate.
 * @param radius Circle radius.
 */
void sdlgfx_circle(int x, int y, int radius) {
    sdlgfx_color(current_color.r, current_color.g, current_color.b);
    int num_segments = 100;
    float angle_step = 2.0 * M_PI / num_segments;
    float prev_x = x + radius;
    float prev_y = y;

    for (int i = 1; i <= num_segments; i++) {
        float angle = i * angle_step;
        float curr_x = x + radius * cos(angle);
        float curr_y = y + radius * sin(angle);
        sdlgfx_line(prev_x, prev_y, curr_x, curr_y);
        prev_x = curr_x;
        prev_y = curr_y;
    }
}

/**
 * @brief Draws a filled circle.
 * @param x Center X coordinate.
 * @param y Center Y coordinate.
 * @param radius Circle radius.
 */
void sdlgfx_fill_circle(int x, int y, int radius) {
    sdlgfx_color(current_color.r, current_color.g, current_color.b);

    for (int i = 0; i <= radius; i++) {
        float angle = acos((float)i / radius);
        int width = radius * sin(angle);
        sdlgfx_line(x - width, y + i, x + width, y + i);
        sdlgfx_line(x - width, y - i, x + width, y - i);
    }
}

/**
 * @brief Draws a triangle outline.
 * @param x1 Vertex 1 X coordinate.
 * @param y1 Vertex 1 Y coordinate.
 * @param x2 Vertex 2 X coordinate.
 * @param y2 Vertex 2 Y coordinate.
 * @param x3 Vertex 3 X coordinate.
 * @param y3 Vertex 3 Y coordinate.
 */
void sdlgfx_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    sdlgfx_line(x1, y1, x2, y2);
    sdlgfx_line(x2, y2, x3, y3);
    sdlgfx_line(x3, y3, x1, y1);
}

/**
 * @brief Draws a filled triangle.
 * @param x1 Vertex 1 X coordinate.
 * @param y1 Vertex 1 Y coordinate.
 * @param x2 Vertex 2 X coordinate.
 * @param y2 Vertex 2 Y coordinate.
 * @param x3 Vertex 3 X coordinate.
 * @param y3 Vertex 3 Y coordinate.
 */
void sdlgfx_fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    // Сортировка вершин по Y
    if (y1 > y2) {
        int tmp_x = x1; x1 = x2; x2 = tmp_x;
        int tmp_y = y1; y1 = y2; y2 = tmp_y;
    }
    if (y1 > y3) {
        int tmp_x = x1; x1 = x3; x3 = tmp_x;
        int tmp_y = y1; y1 = y3; y3 = tmp_y;
    }
    if (y2 > y3) {
        int tmp_x = x2; x2 = x3; x3 = tmp_x;
        int tmp_y = y2; y2 = y3; y3 = tmp_y;
    }

    // Если треугольник вырожденный
    if (y1 == y3) return;

    // Вычисление наклона
    float dx_left, dx_right;
    int x_left, x_right;

    if (y2 - y1 != 0) {
        float slope1 = (float)(x2 - x1) / (y2 - y1);
        float slope2 = (float)(x3 - x1) / (y3 - y1);

        for (int y = y1; y <= y2; y++) {
            x_left = x1 + (y - y1) * slope1;
            x_right = x1 + (y - y1) * slope2;
            if (x_left > x_right) {
                int tmp = x_left; x_left = x_right; x_right = tmp;
            }
            sdlgfx_line(x_left, y, x_right, y);
        }
    }

    if (y3 - y2 != 0) {
        float slope1 = (float)(x3 - x2) / (y3 - y2);
        float slope2 = (float)(x3 - x1) / (y3 - y1);

        for (int y = y2; y <= y3; y++) {
            x_left = x2 + (y - y2) * slope1;
            x_right = x1 + (y - y1) * slope2;
            if (x_left > x_right) {
                int tmp = x_left; x_left = x_right; x_right = tmp;
            }
            sdlgfx_line(x_left, y, x_right, y);
        }
    }
}

/**
 * @brief Draws a filled triangle with a gradient fill between three vertex colors.
 * @param x1, y1 The coordinates of the first vertex.
 * @param r1, g1, b1 The color of the first vertex (0-255).
 * @param x2, y2 The coordinates of the second vertex.
 * @param r2, g2, b2 The color of the second vertex (0-255).
 * @param x3, y3 The coordinates of the third vertex.
 * @param r3, g3, b3 The color of the third vertex (0-255).
 */
void sdlgfx_gradient_triangle(int x1, int y1, int r1, int g1, int b1,
                             int x2, int y2, int r2, int g2, int b2,
                             int x3, int y3, int r3, int g3, int b3) {
    // Сортировка вершин по Y для упрощения заполнения
    if (y1 > y2) {
        int tmp_x = x1; x1 = x2; x2 = tmp_x;
        int tmp_y = y1; y1 = y2; y2 = tmp_y;
        int tmp_r = r1; r1 = r2; r2 = tmp_r;
        int tmp_g = g1; g1 = g2; g2 = tmp_g;
        int tmp_b = b1; b1 = b2; b2 = tmp_b;
    }
    if (y1 > y3) {
        int tmp_x = x1; x1 = x3; x3 = tmp_x;
        int tmp_y = y1; y1 = y3; y3 = tmp_y;
        int tmp_r = r1; r1 = r3; r3 = tmp_r;
        int tmp_g = g1; g1 = g3; g3 = tmp_g;
        int tmp_b = b1; b1 = b3; b3 = tmp_b;
    }
    if (y2 > y3) {
        int tmp_x = x2; x2 = x3; x3 = tmp_x;
        int tmp_y = y2; y2 = y3; y3 = tmp_y;
        int tmp_r = r2; r2 = r3; r3 = tmp_r;
        int tmp_g = g2; g2 = g3; g3 = tmp_g;
        int tmp_b = b2; b2 = b3; b3 = tmp_b;
    }

    // Если треугольник вырожденный
    if (y1 == y3) return;

    // Первая половина треугольника (y1 -> y2)
    if (y2 - y1 != 0) {
        float slope1 = (float)(x2 - x1) / (y2 - y1);
        float slope2 = (float)(x3 - x1) / (y3 - y1);
        for (int y = y1; y <= y2; y++) {
            int x_left = x1 + (y - y1) * slope1;
            int x_right = x1 + (y - y1) * slope2;
            float t1 = (float)(y - y1) / (y2 - y1); // Интерполяция для левой стороны
            float t2 = (float)(y - y1) / (y3 - y1); // Интерполяция для правой стороны
            int r_left = r1 + (r2 - r1) * t1;
            int g_left = g1 + (g2 - g1) * t1;
            int b_left = b1 + (b2 - b1) * t1;
            int r_right = r1 + (r3 - r1) * t2;
            int g_right = g1 + (g3 - g1) * t2;
            int b_right = b1 + (b3 - b1) * t2;

            if (x_left > x_right) {
                int tmp_x = x_left; x_left = x_right; x_right = tmp_x;
                int tmp_r = r_left; r_left = r_right; r_right = tmp_r;
                int tmp_g = g_left; g_left = g_right; g_right = tmp_g;
                int tmp_b = b_left; b_left = b_right; b_right = tmp_b;
            }

            // Интерполяция цвета по горизонтали - SSE2 version
            if ((x_right - x_left + 1) >= 4) {
                int num_pixels = x_right - x_left + 1;
                int num_vector_pixels = num_pixels / 4 * 4;
                float t_step = 1.0f / (x_right - x_left);

                __m128 left_r = _mm_set1_ps((float)r_left);
                __m128 left_g = _mm_set1_ps((float)g_left);
                __m128 left_b = _mm_set1_ps((float)b_left);
                __m128 diff_r = _mm_set1_ps((float)(r_right - r_left));
                __m128 diff_g = _mm_set1_ps((float)(g_right - g_left));
                __m128 diff_b = _mm_set1_ps((float)(b_right - b_left));

                float initial_t = 0.0f;
                for (int x_vec = 0; x_vec < num_vector_pixels; x_vec += 4) {
                    __m128 t_vec = _mm_set_ps(initial_t + 3*t_step, initial_t + 2*t_step, initial_t + t_step, initial_t);
                    __m128 r_vec = _mm_add_ps(left_r, _mm_mul_ps(diff_r, t_vec));
                    __m128 g_vec = _mm_add_ps(left_g, _mm_mul_ps(diff_g, t_vec));
                    __m128 b_vec = _mm_add_ps(left_b, _mm_mul_ps(diff_b, t_vec));

                    float r_array[4], g_array[4], b_array[4];
                    _mm_storeu_ps(r_array, r_vec);
                    _mm_storeu_ps(g_array, g_vec);
                    _mm_storeu_ps(b_array, b_vec);

                    for(int i=0; i<4; ++i) {
                        int r_pixel = (int)r_array[i];
                        int g_pixel = (int)g_array[i];
                        int b_pixel = (int)b_array[i];
                        sdlgfx_color(r_pixel, g_pixel, b_pixel);
                        sdlgfx_point(x_left + x_vec + i, y);
                    }
                    initial_t += 4*t_step;
                }
                // Scalar processing for remaining pixels
                for (int x = x_left + num_vector_pixels; x <= x_right; x++) {
                    float t = (float)(x - x_left) / (x_right - x_left);
                    int r = r_left + (r_right - r_left) * t;
                    int g = g_left + (g_right - g_left) * t;
                    int b = b_left + (b_right - b_left) * t;
                    sdlgfx_color(r, g, b);
                    sdlgfx_point(x, y);
                }


            } else {
                // Scalar processing for short lines
                for (int x = x_left; x <= x_right; x++) {
                    float t = (float)(x - x_left) / (x_right - x_left);
                    int r = r_left + (r_right - r_left) * t;
                    int g = g_left + (g_right - g_left) * t;
                    int b = b_left + (b_right - b_left) * t;
                    sdlgfx_color(r, g, b);
                    sdlgfx_point(x, y);
                }
            }
        }
    }

    // Вторая половина треугольника (y2 -> y3)
    if (y3 - y2 != 0) {
        float slope1 = (float)(x3 - x2) / (y3 - y2);
        float slope2 = (float)(x3 - x1) / (y3 - y1);
        for (int y = y2; y <= y3; y++) {
            int x_left = x2 + (y - y2) * slope1;
            int x_right = x1 + (y - y1) * slope2;
            float t1 = (float)(y - y2) / (y3 - y2);
            float t2 = (float)(y - y1) / (y3 - y1);
            int r_left = r2 + (r3 - r2) * t1;
            int g_left = g2 + (g3 - g2) * t1;
            int b_left = b2 + (b3 - b2) * t1;
            int r_right = r1 + (r3 - r1) * t2;
            int g_right = g1 + (g3 - g1) * t2;
            int b_right = b1 + (b3 - b1) * t2;

            if (x_left > x_right) {
                int tmp_x = x_left; x_left = x_right; x_right = tmp_x;
                int tmp_r = r_left; r_left = r_right; r_right = tmp_r;
                int tmp_g = g_left; g_left = g_right; g_right = tmp_g;
                int tmp_b = b_left; b_left = b_right; b_right = tmp_b;
            }
            // Интерполяция цвета по горизонтали - SSE2 version
            if ((x_right - x_left + 1) >= 4) {
                int num_pixels = x_right - x_left + 1;
                int num_vector_pixels = num_pixels / 4 * 4;
                float t_step = 1.0f / (x_right - x_left);

                __m128 left_r = _mm_set1_ps((float)r_left);
                __m128 left_g = _mm_set1_ps((float)g_left);
                __m128 left_b = _mm_set1_ps((float)b_left);
                __m128 diff_r = _mm_set1_ps((float)(r_right - r_left));
                __m128 diff_g = _mm_set1_ps((float)(g_right - g_left));
                __m128 diff_b = _mm_set1_ps((float)(b_right - b_left));

                float initial_t = 0.0f;
                for (int x_vec = 0; x_vec < num_vector_pixels; x_vec += 4) {
                    __m128 t_vec = _mm_set_ps(initial_t + 3*t_step, initial_t + 2*t_step, initial_t + t_step, initial_t);
                    __m128 r_vec = _mm_add_ps(left_r, _mm_mul_ps(diff_r, t_vec));
                    __m128 g_vec = _mm_add_ps(left_g, _mm_mul_ps(diff_g, t_vec));
                    __m128 b_vec = _mm_add_ps(left_b, _mm_mul_ps(diff_b, t_vec));

                    float r_array[4], g_array[4], b_array[4];
                    _mm_storeu_ps(r_array, r_vec);
                    _mm_storeu_ps(g_array, g_vec);
                    _mm_storeu_ps(b_array, b_vec);

                    for(int i=0; i<4; ++i) {
                        int r_pixel = (int)r_array[i];
                        int g_pixel = (int)g_array[i];
                        int b_pixel = (int)b_array[i];
                        sdlgfx_color(r_pixel, g_pixel, b_pixel);
                        sdlgfx_point(x_left + x_vec + i, y);
                    }
                    initial_t += 4*t_step;
                }
                // Scalar processing for remaining pixels
                for (int x = x_left + num_vector_pixels; x <= x_right; x++) {
                    float t = (float)(x - x_left) / (x_right - x_left);
                    int r = r_left + (r_right - r_left) * t;
                    int g = g_left + (g_right - g_left) * t;
                    int b = b_left + (b_right - b_left) * t;
                    sdlgfx_color(r, g, b);
                    sdlgfx_point(x, y);
                }


            } else {
                // Scalar processing for short lines
                for (int x = x_left; x <= x_right; x++) {
                    float t = (float)(x - x_left) / (x_right - x_left);
                    int r = r_left + (r_right - r_left) * t;
                    int g = g_left + (g_right - g_left) * t;
                    int b = b_left + (b_right - b_left) * t;
                    sdlgfx_color(r, g, b);
                    sdlgfx_point(x, y);
                }
            }
        }
    }
}

/**
 * @brief Draws an ellipse outline.
 * @param x Center X coordinate.
 * @param y Center Y coordinate.
 * @param rx Horizontal radius.
 * @param ry Vertical radius.
 */
void sdlgfx_ellipse(int x, int y, int rx, int ry) {
    sdlgfx_color(current_color.r, current_color.g, current_color.b);
    int num_segments = 100;
    float angle_step = 2.0 * M_PI / num_segments;
    float prev_x = x + rx;
    float prev_y = y;

    for (int i = 1; i <= num_segments; i++) {
        float angle = i * angle_step;
        float curr_x = x + rx * cos(angle);
        float curr_y = y + ry * sin(angle);
        sdlgfx_line(prev_x, prev_y, curr_x, curr_y);
        prev_x = curr_x;
        prev_y = curr_y;
    }
}

/**
 * @brief Draws a filled ellipse.
 * @param x Center X coordinate.
 * @param y Center Y coordinate.
 * @param rx Horizontal radius.
 * @param ry Vertical radius.
 */
void sdlgfx_fill_ellipse(int x, int y, int rx, int ry) {
    sdlgfx_color(current_color.r, current_color.g, current_color.b);
    float aspect_ratio = (float)ry / rx;
    for (int i = 0; i <= ry; i++) {
        float ellipse_y = (float)i / ry;
        int width = rx * sqrt(1.0 - ellipse_y * ellipse_y);
        sdlgfx_line(x - width, y + i, x + width, y + i);
        sdlgfx_line(x - width, y - i, x + width, y - i);
    }
}

/**
 * @brief Draws an arc (part of a circle outline).
 * @param x Center X coordinate.
 * @param y Center Y coordinate.
 * @param radius Arc radius.
 * @param start_angle Start angle in radians.
 * @param end_angle End angle in radians.
 */
void sdlgfx_arc(int x, int y, int radius, float start_angle, float end_angle) {
    sdlgfx_color(current_color.r, current_color.g, current_color.b);
    int num_segments = 100;
    float angle_step = (end_angle - start_angle) / num_segments;
    float prev_x = x + radius * cos(start_angle);
    float prev_y = y + radius * sin(start_angle);

    for (int i = 1; i <= num_segments; i++) {
        float angle = start_angle + i * angle_step;
        float curr_x = x + radius * cos(angle);
        float curr_y = y + radius * sin(angle);
        sdlgfx_line(prev_x, prev_y, curr_x, curr_y);
        prev_x = curr_x;
        prev_y = curr_y;
    }
}


/**
 * @brief Draws text string using built-in font.
 * @param x Top-left X coordinate.
 * @param y Top-left Y coordinate.
 * @param cc Text string to draw.
 */
void sdlgfx_string(int x, int y, const char *cc) {
    sdlfont_draw_string(x, y, cc, sdlgfx_renderer);
}

/**
 * @brief Flushes the rendering buffer to display.
 */
void sdlgfx_flush(void) {
    SDL_RenderPresent(sdlgfx_renderer);
}

/**
 * @brief Waits for user input (key press or quit).
 */
void sdlgfx_wait(void) {
    SDL_Event event;
    int done = 0;

    while (!done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                case SDL_KEYDOWN:
                    done = 1;
                    break;
            }
        }

        SDL_Delay(10);
    }
}

/**
 * @brief Checks if there are pending SDL events.
 * @return 1 if events are waiting, 0 otherwise.
 */
int sdlgfx_event_waiting(void) {
    return SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) > 0;
}

/**
 * @brief Returns the window width.
 * @return Window width in pixels.
 */
int sdlgfx_xsize(void) {
    return window_width;
}

/**
 * @brief Returns the window height.
 * @return Window height in pixels.
 */
int sdlgfx_ysize(void) {
    return window_height;
}

/**
 * @brief Moves the window to absolute coordinates.
 * @param x New X coordinate.
 * @param y New Y coordinate.
 */
void sdlgfx_move_win_abs(int x, int y) {
    if (sdlgfx_window) {
        SDL_SetWindowPosition(sdlgfx_window, x, y);
    }
}

/**
 * @brief Moves the window relative to its current position.
 * @param dx X offset.
 * @param dy Y offset.
 */
void sdlgfx_move_win_rel(int dx, int dy) {
    if (sdlgfx_window) {
        int x, y;
        SDL_GetWindowPosition(sdlgfx_window, &x, &y);
        SDL_SetWindowPosition(sdlgfx_window, x + dx, y + dy);
    }
}

/**
 * @brief Gets the color of a pixel at given coordinates.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return Pixel color as 0xRRGGBB integer.
 */
int GetPix(int x, int y) {
    if (!sdlgfx_renderer || !sdlgfx_window) {
        fprintf(stderr, "GetPix: Renderer or window not initialized.\n");
        return 0;
    }

    if (x < 0 || x >= window_width || y < 0 || y >= window_height) { return 0; }

    SDL_Surface *temp_surface = SDL_CreateRGBSurfaceWithFormat(0, window_width, window_height, 32, SDL_PIXELFORMAT_ARGB8888);

    if (!temp_surface) {
        fprintf(stderr, "GetPix: SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
        return 0;
    }

    int result = SDL_RenderReadPixels(sdlgfx_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, temp_surface->pixels, temp_surface->pitch);

    if (result != 0) {
        fprintf(stderr, "GetPix: SDL_RenderReadPixels failed: %s\n", SDL_GetError());
        SDL_FreeSurface(temp_surface);
        return 0;
    }

    Uint32 *pixels = (Uint32 *)temp_surface->pixels;
    int pixel_offset = (y * temp_surface->pitch / 4) + x;
    Uint32 pixel_color = pixels[pixel_offset];

    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel_color, temp_surface->format, &r, &g, &b, &a);
    SDL_FreeSurface(temp_surface);

    return ((r << 16) | (g << 8) | b);
}

/**
 * @brief Draws a single pixel.
 * @param x X coordinate.
 * @param y Y coordinate.
 */
void sdlgfx_pixel(int x, int y) {
    SDL_RenderDrawPoint(sdlgfx_renderer, x, y);
}

/**
 * @brief Draws a pixel with gradient color.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 */
void sdlgfx_pixel_gradient(int x, int y, int r, int g, int b) {
    sdlgfx_color(r, g, b);
    SDL_RenderDrawPoint(sdlgfx_renderer, x, y);
}

/**
 * @brief Draws an extended vertical gradient.
 * @param r1,g1,b1 Top color.
 * @param r2,g2,b2 Second color.
 * @param r3,g3,b3 Third color.
 * @param r4,g4,b4 Bottom color.
 * @param num_stops Number of color stops (2-4).
 * @param width Gradient width.
 * @param height Gradient height.
 * @param scanlines_enabled Enable scanlines effect (1/0).
 * @param noise_enabled Enable line noise effect (1/0).
 * @param pixel_noise_enabled Enable pixel noise effect (1/0).
 */
void sdlgfx_gradient_vertical_ex(int r1, int g1, int b1, int r2, int g2, int b2, int r3, int g3, int b3, int r4, int g4, int b4, int num_stops, int width, int height, int scanlines_enabled, int noise_enabled, int pixel_noise_enabled) {
    int num_actual_stops = num_stops;

    if (num_actual_stops < 2 || num_actual_stops > 4) {
        num_actual_stops = 2; // Default to 2 stops if num_stops is out of range
    }

    for (int y = 0; y < height; y++) {
        float t;
        int r, g, b;

        if (num_actual_stops == 4) {
            int section_height = height / 3; // Divide into 3 sections for 4 stops

            if (y < section_height) {
                t = (float)y / section_height;
                r = (int)(r1 + (r2 - r1) * t);
                g = (int)(g1 + (g2 - g1) * t);
                b = (int)(b1 + (b2 - b1) * t);
            } else if (y < 2 * section_height) {
                t = (float)(y - section_height) / section_height;
                r = (int)(r2 + (r3 - r2) * t);
                g = (int)(g2 + (g3 - g2) * t);
                b = (int)(b2 + (b3 - b2) * t);
            } else {
                t = (float)(y - 2 * section_height) / (height - 2 * section_height);
                r = (int)(r3 + (r4 - r3) * t);
                g = (int)(g3 + (g4 - g3) * t);
                b = (int)(b3 + (b4 - b3) * t);
            }
        } else if (num_actual_stops == 3) {
            int half_height = height / 2;
            if (y < half_height) {
                t = (float)y / half_height;
                r = (int)(r1 + (r2 - r1) * t);
                g = (int)(g1 + (g2 - g1) * t);
                b = (int)(b1 + (b2 - b1) * t);
            } else {
                t = (float)(y - half_height) / half_height;
                r = (int)(r2 + (r3 - r2) * t);
                g = (int)(g2 + (g3 - g2) * t);
                b = (int)(b2 + (b3 - b2) * t);
            }
        } else { // num_actual_stops == 2
            t = (float)y / (height - 1);
            r = (int)(r1 + (r2 - r1) * t);
            g = (int)(g1 + (g2 - g1) * t);
            b = (int)(b1 + (b2 - b1) * t);
        }

        if (noise_enabled) {
            float noise_factor = 0.05f;
            r += (int)((float)(rand() % 255 - 128) * noise_factor);
            g += (int)((float)(rand() % 255 - 128) * noise_factor);
            b += (int)((float)(rand() % 255 - 128) * noise_factor);
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;
        }

        if (pixel_noise_enabled) {
            int noise = (rand() % 41) - 20;
            r = (r + noise) < 0 ? 0 : (r + noise) > 255 ? 255 : r + noise;
            g = (g + noise) < 0 ? 0 : (g + noise) > 255 ? 255 : g + noise;
            b = (b + noise) < 0 ? 0 : (b + noise) > 255 ? 255 : b + noise;
        }

        sdlgfx_color(r, g, b);
        SDL_RenderDrawLine(sdlgfx_renderer, 0, y, width - 1, y);

        if (scanlines_enabled && (y % 2 == 0)) {
            sdlgfx_color(0, 0, 0);
            SDL_RenderDrawLine(sdlgfx_renderer, 0, y, width - 1, y);
        }
    }
}

/**
 * @brief Checks if a character pixel is set in the font bitmap.
 * @param text The text string.
 * @param text_cols The number of columns in the text layout (usually string length).
 * @param char_index_x The horizontal character index.
 * @param pixel_x_in_char The x-coordinate of the pixel within the character (0-FONT_WIDTH-1).
 * @param char_index_y The vertical character index (usually 0 for single-line text).
 * @param pixel_y_in_char The y-coordinate of the pixel within the character (0-FONT_HEIGHT-1).
 * @return SDL_TRUE if the pixel is set, SDL_FALSE otherwise.
 */
SDL_bool sdlgfx_is_char_pixel(const char* text, int text_cols, int char_index_x, int pixel_x_in_char, int char_index_y, int pixel_y_in_char) {
    if (char_index_x < 0 || char_index_x >= text_cols) return SDL_FALSE;
    if (pixel_x_in_char < 0 || pixel_x_in_char >= FONT_WIDTH) return SDL_FALSE;
    if (char_index_y < 0 || char_index_y >= 1 ) return SDL_FALSE; // Assuming single line text for MUTE
    if (pixel_y_in_char < 0 || pixel_y_in_char >= FONT_HEIGHT) return SDL_FALSE;

    char c = text[char_index_x];
    FontBitmap bitmap;
    sdlfont_generate_char_bitmap(bitmap, c);
    return ((bitmap[pixel_y_in_char] >> (7 - pixel_x_in_char)) & 0x01) ? SDL_TRUE : SDL_FALSE;
}

/**
 * @brief Gets the SDL window handle used by the library.
 *
 * @return Pointer to the SDL_Window used by sdlgfx.
 */
SDL_Window* sdlgfx_get_window(void) {
    return sdlgfx_window;
}



/* ====================================================================== */
/*                  	TEST	TEST	TEST                              */
/* ====================================================================== */

/**
 * @brief Locks the streaming texture for direct pixel access.
 *
 * This function must be called before directly manipulating pixels of a streaming texture.
 * It returns a pointer to the pixel data. Make sure to call sdlgfx_unlock_texture_pixels()
 * after pixel manipulation is complete.
 *
 * @param pitch Pointer to store the pitch (in pixels) of the texture.
 * @return Pointer to the pixel data of the streaming texture, or NULL on error.
 */
void* sdlgfx_lock_texture_pixels(int *pitch) {
    if (!sdlgfx_renderer) {
        fprintf(stderr, "sdlgfx_lock_texture_pixels: Renderer is not initialized.\n");
        return NULL;
    }
    if (!use_streaming_texture) {
        fprintf(stderr, "sdlgfx_lock_texture_pixels: Streaming texture is not enabled.\n");
        return NULL;
    }
    if (!sdlgfx_texture) {
        fprintf(stderr, "sdlgfx_lock_texture_pixels: Texture is not created.\n");
        return NULL;
    }
    if (locked_pixels) {
        fprintf(stderr, "sdlgfx_lock_texture_pixels: Texture is already locked. Unlock it first.\n");
        return NULL; // Можно изменить на return locked_pixels, если повторный вызов допустим
    }

    if (SDL_LockTexture(sdlgfx_texture, NULL, &locked_pixels, &locked_pitch) != 0) {
        fprintf(stderr, "sdlgfx_lock_texture_pixels: SDL_LockTexture Error: %s\n", SDL_GetError());
        locked_pixels = NULL;
        locked_pitch = 0;
        return NULL;
    }

    // Проверка формата текстуры
    Uint32 format;
    int access, w, h;
    if (SDL_QueryTexture(sdlgfx_texture, &format, &access, &w, &h) != 0) {
        fprintf(stderr, "sdlgfx_lock_texture_pixels: SDL_QueryTexture Error: %s\n", SDL_GetError());
        SDL_UnlockTexture(sdlgfx_texture);
        locked_pixels = NULL;
        locked_pitch = 0;
        return NULL;
    }
    if (format != SDL_PIXELFORMAT_RGBA8888) {
        fprintf(stderr, "sdlgfx_lock_texture_pixels: Unsupported texture format. Expected RGBA8888.\n");
        SDL_UnlockTexture(sdlgfx_texture);
        locked_pixels = NULL;
        locked_pitch = 0;
        return NULL;
    }

    *pitch = locked_pitch / sizeof(Uint32); // Pitch в пикселях для RGBA8888
    return locked_pixels;
}

/**
 * @brief Unlocks the streaming texture, applying pixel changes.
 *
 * This function must be called after sdlgfx_lock_texture_pixels() and after pixel
 * manipulation is complete to unlock the texture and update the rendering.
 */
void sdlgfx_unlock_texture_pixels(void) {
    if (!locked_pixels) {
        fprintf(stderr, "sdlgfx_unlock_texture_pixels: Texture is not locked.\n");
        return;
    }
    if (!sdlgfx_texture) {
        fprintf(stderr, "sdlgfx_unlock_texture_pixels: Texture is not created.\n");
        return;
    }
    SDL_UnlockTexture(sdlgfx_texture);
    locked_pixels = NULL;
    locked_pitch = 0;
}

