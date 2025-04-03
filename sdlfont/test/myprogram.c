#include <SDL2/SDL.h>
#include "sdlgfx.h"
#include "sdlfont.h"
#include <stdio.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

void set_text_color(uint8_t r, uint8_t g, uint8_t b, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

int main() {
    // Инициализация окна
    sdlgfx_open(WINDOW_WIDTH, WINDOW_HEIGHT, "Advanced Font Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    if (!sdlgfx_renderer) { // Проверка глобальной переменной
        printf("Failed to initialize SDL window\n");
        return 1;
    }

    sdlgfx_clear_color(0, 0, 0);
    sdlgfx_clear();

    sdlfont_init();

    if (!sdlfont_load_psf("Unifont-APL8x16-16.0.02.psf")) {
        printf("No PSF font loaded, using built-in or Unicode\n");
    }

    const char *test_strings[] = {
        "Hello, World!",
        "Привет, Мир!",
        "ASCII: !\"#$%&'()*+",
        "Кириллица: АБВГДЕЁЖЗ",
        "Mixed: Hello Привет 123"
    };
    int num_strings = sizeof(test_strings) / sizeof(test_strings[0]);

    uint8_t colors[][3] = {
        {255, 255, 255},
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
        {255, 255, 0}
    };

    int y_pos = 50;
    for (int i = 0; i < num_strings; i++) {
        set_text_color(colors[i][0], colors[i][1], colors[i][2], sdlgfx_renderer);
        sdlfont_draw_string(50, y_pos, test_strings[i], sdlgfx_renderer);
        y_pos += FONT_HEIGHT + 10;
    }

    set_text_color(128, 128, 128, sdlgfx_renderer);
    int text_width = 400;
    int text_height = num_strings * (FONT_HEIGHT + 10);
    SDL_Rect border = {40, 40, text_width + 20, text_height + 20};
    SDL_RenderDrawRect(sdlgfx_renderer, &border);

    sdlgfx_flush();

    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }
        SDL_Delay(16);
    }

    sdlgfx_close();
    return 0;
}
