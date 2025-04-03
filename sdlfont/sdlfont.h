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

/**
 * sdlfont.h - Header file for font management in SDLGFX.
 */


#ifndef SDLFONT_H
#define SDLFONT_H

#include <SDL2/SDL.h>
#include <stdint.h>

#define FONT_WIDTH  8
#define FONT_HEIGHT 16

typedef uint8_t FontBitmap[FONT_HEIGHT];

void sdlfont_init(void);
int  sdlfont_load_psf(const char *path);
void sdlfont_generate_char_bitmap(FontBitmap bitmap, uint32_t c);
void sdlfont_draw_char(int x, int y, FontBitmap bitmap, SDL_Renderer *renderer);
void sdlfont_draw_string(int x, int y, const char *str, SDL_Renderer *renderer);

#endif
