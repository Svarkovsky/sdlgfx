/*
    gcc -std=c11 -o sdlgfx_demo_functions sdlgfx_demo_functions.c sdlgfx.c sdlfont.c -lSDL2 -lm

    gcc -std=c11 -pedantic-errors -o sdlgfx_demo_functions sdlgfx_demo_functions.c sdlgfx.c sdlfont.c \
    -lSDL2 -lm \
    -Ofast -flto=$(nproc) \
    -march=native -mtune=native \
    -mfpmath=sse -msse3 -mssse3 \
    -fopenmp \
    -falign-functions=16 -falign-loops=16 -fomit-frame-pointer -fno-ident -fno-asynchronous-unwind-tables -fvisibility=hidden -fno-plt \
    -ftree-vectorize -fopt-info-vec \
    -fipa-pta -fipa-icf -fipa-cp-clone -funroll-loops -floop-interchange -fgraphite-identity -floop-nest-optimize -fmerge-all-constants \
    -fvariable-expansion-in-unroller \
    -fno-stack-protector \
    -Wl,-z,norelro \
    -s -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,--strip-all \
    -pipe -DNDEBUG \
    -DUSE_UNICODE

*/

#include <SDL2/SDL.h>

#include "sdlgfx.h"
#include "sdlfont.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <immintrin.h>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540
#define DEMO_DURATION 5000
#define MOVE_SPEED 4
#define ROTATION_SPEED 0.05      // 0.05
#define INFO_PANEL_HEIGHT 30    // 50

#ifndef M_PI
#define M_PI acos(-1.0)
#endif

typedef struct {
    int x, y;
    float dx, dy;
    int size;
    int r, g, b;
    float rotation;
} MovingObject;

static int running = 1;

void handle_input(int* running);
void draw_info_panel(const char* func_name, const char* description, int demo_time);
void update_position(MovingObject* obj, int width, int height);
void demo_triangles(void);
void demo_circles(void);
void demo_rectangles(void);
void demo_lines(void);
void demo_gradients(void);
void demo_window_movement(void);
void demo_points(void);
void demo_text(void);
void demo_ellipses(void);
void demo_arcs(void);
//void demo_gradient_triangles(void);
void demo_pixel_effects(void);
void demo_text_collision(void);

void draw_info_panel(const char* func_name, const char* description, int demo_time) {
    sdlgfx_color(0, 0, 0);
    sdlgfx_fill_rectangle(0, 0, SCREEN_WIDTH, INFO_PANEL_HEIGHT);
    
    sdlgfx_color(255, 255, 255);
    char info_text[256];
    snprintf(info_text, sizeof(info_text), "FUNCTION: %s | %s | Time: %ds | Press Q/Esc to exit",
             func_name, description, demo_time/1000);
    sdlgfx_string(10, 10, info_text);
}

void update_position(MovingObject* obj, int width, int height) {
    obj->x += obj->dx;
    obj->y += obj->dy;
    obj->rotation += ROTATION_SPEED;

    if (obj->x < obj->size) {
        obj->x = obj->size;
        obj->dx = -obj->dx * 0.9f;
    }
    if (obj->x > width - obj->size) {
        obj->x = width - obj->size;
        obj->dx = -obj->dx * 0.9f;
    }
    if (obj->y < obj->size + INFO_PANEL_HEIGHT) {
        obj->y = obj->size + INFO_PANEL_HEIGHT;
        obj->dy = -obj->dy * 0.9f;
    }
    if (obj->y > height - obj->size) {
        obj->y = height - obj->size;
        obj->dy = -obj->dy * 0.9f;
    }
}

void demo_triangles() {
    sdlgfx_clear();
    MovingObject triangles[5];
    Uint32 start_time = SDL_GetTicks();

    for (int i = 0; i < 5; i++) {
        triangles[i].x = SCREEN_WIDTH/2 + (i-2)*150;
        triangles[i].y = SCREEN_HEIGHT/2;
        triangles[i].dx = ((float)rand() / RAND_MAX * 6.0f) - 3.0f;
        triangles[i].dy = ((float)rand() / RAND_MAX * 6.0f) - 3.0f;
        triangles[i].size = 30 + i*20;
        triangles[i].r = rand() % 256;
        triangles[i].g = rand() % 256;
        triangles[i].b = rand() % 256;
        triangles[i].rotation = 0;
    }

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        for (int i = 0; i < 5; i++) {
            update_position(&triangles[i], SCREEN_WIDTH, SCREEN_HEIGHT);
            float s = sin(triangles[i].rotation);
            float c = cos(triangles[i].rotation);
            int size = triangles[i].size;
            int x1 = triangles[i].x + (int)(size * c);
            int y1 = triangles[i].y + (int)(size * s);
            int x2 = triangles[i].x + (int)(size * cos(triangles[i].rotation + 2*M_PI/3));
            int y2 = triangles[i].y + (int)(size * sin(triangles[i].rotation + 2*M_PI/3));
            int x3 = triangles[i].x + (int)(size * cos(triangles[i].rotation + 4*M_PI/3));
            int y3 = triangles[i].y + (int)(size * sin(triangles[i].rotation + 4*M_PI/3));

            if (i % 3 == 0) {
                sdlgfx_color(triangles[i].r, triangles[i].g, triangles[i].b);
                sdlgfx_fill_triangle(x1, y1, x2, y2, x3, y3);
            } else if (i % 3 == 1) {
                for (int y = (y1 < y2 ? y1 : y2); y <= (y3 > y2 ? y3 : y2); y++) {
                    int r = triangles[i].r + (y - y1) * 20 % 255;
                    int g = triangles[i].g + (y - y1) * 30 % 255;
                    int b = triangles[i].b + (y - y1) * 40 % 255;
                    sdlgfx_color(r, g, b);
                    sdlgfx_line(x1 + (y-y1)*0.5, y, x2 - (y-y2)*0.5, y);
                }
            } else {
                sdlgfx_color(triangles[i].r, triangles[i].g, triangles[i].b);
                sdlgfx_triangle(x1, y1, x2, y2, x3, y3);
            }
        }

        draw_info_panel("sdlgfx_triangle", "Rotating Gradient Triangles",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_circles() {
    sdlgfx_clear();
    MovingObject circles[5];
    Uint32 start_time = SDL_GetTicks();

    for (int i = 0; i < 5; i++) {
        circles[i].x = SCREEN_WIDTH/2 + (i-2)*150;
        circles[i].y = SCREEN_HEIGHT/2;
        circles[i].dx = ((float)rand() / RAND_MAX * 6.0f) - 3.0f;
        circles[i].dy = ((float)rand() / RAND_MAX * 6.0f) - 3.0f;
        circles[i].size = 20 + i*15;
        circles[i].r = rand() % 256;
        circles[i].g = rand() % 256;
        circles[i].b = rand() % 256;
        circles[i].rotation = 0;
    }

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        for (int i = 0; i < 5; i++) {
            update_position(&circles[i], SCREEN_WIDTH, SCREEN_HEIGHT);
            int radius = circles[i].size + (int)(sin(circles[i].rotation) * 10);
            
            if (i % 2 == 0) {
                sdlgfx_color(circles[i].r, circles[i].g, circles[i].b);
                sdlgfx_fill_circle(circles[i].x, circles[i].y, radius);
            } else {
                sdlgfx_color(circles[i].r, circles[i].g, circles[i].b);
                sdlgfx_circle(circles[i].x, circles[i].y, radius);
            }
        }

        draw_info_panel("sdlgfx_circle", "Pulsating Moving Circles",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_rectangles() {
    sdlgfx_clear();
    MovingObject rects[5];
    Uint32 start_time = SDL_GetTicks();

    for (int i = 0; i < 5; i++) {
        rects[i].x = SCREEN_WIDTH/2 + (i-2)*150;
        rects[i].y = SCREEN_HEIGHT/2;
        rects[i].dx = ((float)rand() / RAND_MAX * 6.0f) - 3.0f;
        rects[i].dy = ((float)rand() / RAND_MAX * 6.0f) - 3.0f;
        rects[i].size = 40 + i*20;
        rects[i].r = rand() % 256;
        rects[i].g = rand() % 256;
        rects[i].b = rand() % 256;
        rects[i].rotation = 0;
    }

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        for (int i = 0; i < 5; i++) {
            update_position(&rects[i], SCREEN_WIDTH, SCREEN_HEIGHT);
            int half_size = rects[i].size/2;
            int x1 = rects[i].x - half_size;
            int y1 = rects[i].y - half_size;
            int x2 = rects[i].x + half_size;
            int y2 = rects[i].y + half_size;

            if (i % 2 == 0) {
                sdlgfx_color(rects[i].r, rects[i].g, rects[i].b);
                sdlgfx_fill_rectangle(x1, y1, x2, y2);
            } else {
                sdlgfx_color(rects[i].r, rects[i].g, rects[i].b);
                sdlgfx_rectangle(x1, y1, x2, y2);
            }
        }

        draw_info_panel("sdlgfx_rectangle", "Rotating Moving Rectangles",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_lines() {
    sdlgfx_clear();
    Uint32 start_time = SDL_GetTicks();
    float rotation_offset = 0;

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        int center_x = SCREEN_WIDTH / 2;
        int center_y = (SCREEN_HEIGHT - INFO_PANEL_HEIGHT) / 2 + INFO_PANEL_HEIGHT;
        
        for (int i = 0; i < 360; i += 15) {
            float angle = i * M_PI / 180.0f + rotation_offset;
            int x = center_x + (int)(200 * cos(angle));
            int y = center_y + (int)(200 * sin(angle));
            
            int r = (int)(127 * sin(angle) + 128);
            int g = (int)(127 * sin(angle + 2*M_PI/3) + 128);
            int b = (int)(127 * sin(angle + 4*M_PI/3) + 128);
            
            sdlgfx_color(r, g, b);
            sdlgfx_line(center_x, center_y, x, y);
        }
        
        rotation_offset += ROTATION_SPEED;

        draw_info_panel("sdlgfx_line", "Rotating Rainbow Lines from Center",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_gradients() {
    sdlgfx_clear();
    Uint32 start_time = SDL_GetTicks();
    int phase = 0;
    int last_phase = -1;

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        Uint32 elapsed = SDL_GetTicks() - start_time;
        
        if (elapsed > (DEMO_DURATION/6) * 5) phase = 5;
        else if (elapsed > (DEMO_DURATION/6) * 4) phase = 4;
        else if (elapsed > (DEMO_DURATION/6) * 3) phase = 3;
        else if (elapsed > (DEMO_DURATION/6) * 2) phase = 2;
        else if (elapsed > DEMO_DURATION/6) phase = 1;
        else phase = 0;

        if (phase != last_phase) {
            sdlgfx_clear();
            switch (phase) {
                case 0:
                    sdlgfx_gradient_vertical_ex(255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
                                              2, SCREEN_WIDTH, SCREEN_HEIGHT - INFO_PANEL_HEIGHT, 0, 0, 0);
                    break;
                case 1:
                    sdlgfx_gradient_vertical_ex(255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0,
                                              3, SCREEN_WIDTH, SCREEN_HEIGHT - INFO_PANEL_HEIGHT, 0, 0, 0);
                    break;
                case 2:
                    sdlgfx_gradient_vertical_ex(255, 0, 0, 255, 255, 0, 0, 255, 0, 0, 0, 255,
                                              4, SCREEN_WIDTH, SCREEN_HEIGHT - INFO_PANEL_HEIGHT, 0, 0, 0);
                    break;
                case 3:
                    sdlgfx_gradient_vertical_ex(255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                              2, SCREEN_WIDTH, SCREEN_HEIGHT - INFO_PANEL_HEIGHT, 1, 0, 0);
                    break;
                case 4:
                    sdlgfx_gradient_vertical_ex(0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0,
                                              2, SCREEN_WIDTH, SCREEN_HEIGHT - INFO_PANEL_HEIGHT, 0, 1, 0);
                    break;
                case 5:
                    sdlgfx_gradient_vertical_ex(255, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
                                              2, SCREEN_WIDTH, SCREEN_HEIGHT - INFO_PANEL_HEIGHT, 0, 0, 1);
                    break;
            }
            last_phase = phase;
        }

        draw_info_panel("sdlgfx_gradient_vertical_ex",
                       phase == 0 ? "2-stop gradient (Red to Blue)" :
                       phase == 1 ? "3-stop gradient (Red-Green-Blue)" :
                       phase == 2 ? "4-stop gradient (Red-Yellow-Green-Blue)" :
                       phase == 3 ? "Gradient with scanlines effect" :
                       phase == 4 ? "Gradient with line noise effect" :
                       "Gradient with pixel noise effect",
                       DEMO_DURATION - elapsed);
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_window_movement() {
    sdlgfx_clear();
    Uint32 start_time = SDL_GetTicks();
    int orig_x = SDL_WINDOWPOS_CENTERED;
    int orig_y = SDL_WINDOWPOS_CENTERED;
    SDL_Window* window = sdlgfx_get_window();

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        float t = (SDL_GetTicks() - start_time) / 1000.0f;
        int x = orig_x + (int)(100 * sin(t));
        int y = orig_y + (int)(50 * cos(t * 1.5f)); 
        SDL_SetWindowPosition(window, x, y);

        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(t) * 50, 
                                  150 + cos(t) * 50, 
                                  150 + sin(t * 2) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);
        draw_info_panel("SDL_SetWindowPosition", "Window Moving Smoothly",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
    SDL_SetWindowPosition(window, orig_x, orig_y);
}

void demo_points() {
    sdlgfx_clear();
    MovingObject points[500];
    Uint32 start_time = SDL_GetTicks();

    for (int i = 0; i < 500; i++) {
        points[i].x = rand() % SCREEN_WIDTH;
        points[i].y = INFO_PANEL_HEIGHT + rand() % (SCREEN_HEIGHT - INFO_PANEL_HEIGHT);
        points[i].dx = ((float)rand() / RAND_MAX * 4.0f) - 2.0f;
        points[i].dy = ((float)rand() / RAND_MAX * 4.0f) - 2.0f;
        points[i].size = 2;
        points[i].r = rand() % 256;
        points[i].g = rand() % 256;
        points[i].b = rand() % 256;
        points[i].rotation = 0;
    }

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        for (int i = 0; i < 500; i++) {
            update_position(&points[i], SCREEN_WIDTH, SCREEN_HEIGHT);
            sdlgfx_color(points[i].r, points[i].g, points[i].b);
            sdlgfx_point(points[i].x, points[i].y);
        }

        draw_info_panel("sdlgfx_point", "Moving Random Colored Points",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_text() {
    sdlgfx_clear();
    Uint32 start_time = SDL_GetTicks();
    float offset = 0;

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        offset += 0.05f;
        int x = SCREEN_WIDTH/2 - 200 + (int)(50 * sin(offset));
        int y_base = SCREEN_HEIGHT/2;

        sdlgfx_color(255, 255, (int)(127 * sin(offset) + 128));
        sdlgfx_string(x, y_base - 60, "This is a test string");
        
        sdlgfx_color(255, (int)(127 * sin(offset) + 128), 0);
        sdlgfx_string(x, y_base - 30, "Red text moving");
        
        sdlgfx_color((int)(127 * cos(offset) + 128), 255, 0);
        sdlgfx_string(x, y_base, "Green text moving");
        
        sdlgfx_color(0, (int)(127 * sin(offset + M_PI) + 128), 255);
        sdlgfx_string(x, y_base + 30, "Blue text moving");
        
        sdlgfx_color((int)(127 * cos(offset) + 128), 255, (int)(127 * sin(offset) + 128));
        sdlgfx_string(x, y_base + 60, "Yellow text moving");

        draw_info_panel("sdlgfx_string", "Animated Colored Text",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_ellipses() {
    sdlgfx_clear();
    MovingObject ellipses[5];
    Uint32 start_time = SDL_GetTicks();

    for (int i = 0; i < 5; i++) {
        ellipses[i].x = SCREEN_WIDTH/2 + (i-2)*150;
        ellipses[i].y = SCREEN_HEIGHT/2;
        ellipses[i].dx = ((float)rand() / RAND_MAX * 6.0f) - 3.0f;
        ellipses[i].dy = ((float)rand() / RAND_MAX * 6.0f) - 3.0f;
        ellipses[i].size = 30 + i*10;
        ellipses[i].r = rand() % 256;
        ellipses[i].g = rand() % 256;
        ellipses[i].b = rand() % 256;
        ellipses[i].rotation = 0;
    }

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        for (int i = 0; i < 5; i++) {
            update_position(&ellipses[i], SCREEN_WIDTH, SCREEN_HEIGHT);
            int rx = ellipses[i].size + (int)(sin(ellipses[i].rotation) * 10);
            int ry = ellipses[i].size / 2 + (int)(cos(ellipses[i].rotation) * 5);
            
            if (i % 2 == 0) {
                sdlgfx_color(ellipses[i].r, ellipses[i].g, ellipses[i].b);
                sdlgfx_fill_ellipse(ellipses[i].x, ellipses[i].y, rx, ry);
            } else {
                sdlgfx_color(ellipses[i].r, ellipses[i].g, ellipses[i].b);
                sdlgfx_ellipse(ellipses[i].x, ellipses[i].y, rx, ry);
            }
        }

        draw_info_panel("sdlgfx_ellipse", "Pulsating Moving Ellipses",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_arcs() {
    sdlgfx_clear();
    Uint32 start_time = SDL_GetTicks();
    float angle_offset = 0;

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        int center_x = SCREEN_WIDTH / 2;
        int center_y = SCREEN_HEIGHT / 2;
        for (int i = 0; i < 5; i++) {
            float start_angle = angle_offset + i * M_PI / 5;
            float end_angle = start_angle + M_PI * (1 + sin(angle_offset));
            int radius = 50 + i * 30;
            int r = (int)(127 * sin(angle_offset + i) + 128);
            int g = (int)(127 * cos(angle_offset + i) + 128);
            int b = (int)(127 * sin(angle_offset + i + M_PI) + 128);
            sdlgfx_color(r, g, b);
            sdlgfx_arc(center_x, center_y, radius, start_angle, end_angle);
        }

        angle_offset += ROTATION_SPEED;
        draw_info_panel("sdlgfx_arc", "Rotating Pulsating Arcs",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void demo_pixel_effects() {
    Uint32 start_time = SDL_GetTicks();
    float time = 0;
    float last_time = -1; // Для отслеживания изменений времени

    // Включаем streaming texture для динамического обновления
    sdlgfx_set_streaming_texture(1);

    // Буфер для хранения предыдущего состояния пикселей (для сглаживания)
    Uint32* pixel_buffer = (Uint32*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Uint32));
    if (!pixel_buffer) {
        fprintf(stderr, "Failed to allocate pixel buffer in demo_pixel_effects.\n");
        return;
    }
    memset(pixel_buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Uint32));

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        time = SDL_GetTicks() / 1000.0f; // Обновляем время плавно

        // Блокируем текстуру для прямого доступа к пикселям
        int pitch;
        Uint32* pixels = (Uint32*)sdlgfx_lock_texture_pixels(&pitch);
        if (!pixels) {
            fprintf(stderr, "Failed to lock texture pixels in demo_pixel_effects.\n");
            break;
        }

        // Обновляем эффект только если прошло достаточно времени (например, каждые 0.1 секунды)
        if (fabs(time - last_time) >= 0.1f) {

#pragma omp parallel for collapse(2) schedule(static, 16)

            for (int y = INFO_PANEL_HEIGHT; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    int r = (int)(128 + 127 * sin(x * 0.05 + time));
                    int g = (int)(128 + 127 * sin(y * 0.05 + time));
                    int b = (int)(128 + 127 * sin((x + y) * 0.03 + time));
                    Uint32 new_color = (255 << 24) | (r << 16) | (g << 8) | b;

                    // Сглаживание: смешиваем с предыдущим значением
                    Uint32 prev_color = pixel_buffer[y * SCREEN_WIDTH + x];
                    int prev_r = (prev_color >> 16) & 0xFF;
                    int prev_g = (prev_color >> 8) & 0xFF;
                    int prev_b = prev_color & 0xFF;
                    r = (int)(0.7 * prev_r + 0.3 * r); // Линейная интерполяция
                    g = (int)(0.7 * prev_g + 0.3 * g);
                    b = (int)(0.7 * prev_b + 0.3 * b);

                    Uint32 smoothed_color = (255 << 24) | (r << 16) | (g << 8) | b;
                    pixel_buffer[y * SCREEN_WIDTH + x] = smoothed_color;
                    pixels[y * pitch + x] = smoothed_color;
                }
            }
            last_time = time;
        } else {
            // Если обновление не требуется, просто копируем буфер в текстуру
            for (int y = INFO_PANEL_HEIGHT; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    pixels[y * pitch + x] = pixel_buffer[y * SCREEN_WIDTH + x];
                }
            }
        }

        // Разблокируем текстуру после изменения пикселей
        sdlgfx_unlock_texture_pixels();

        // Очищаем рендерер перед отрисовкой
        SDL_SetRenderTarget(sdlgfx_renderer, NULL);
        SDL_SetRenderDrawColor(sdlgfx_renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlgfx_renderer);

        // Копируем текстуру на экран
        SDL_Rect dstRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(sdlgfx_renderer, sdlgfx_texture, NULL, &dstRect);

        // Рисуем информационную панель
        draw_info_panel("sdlgfx_pixel_gradient", "Plasma Effect with Streaming Texture",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }

    sdlgfx_set_streaming_texture(0); // Отключаем обратно
    free(pixel_buffer); // Освобождаем буфер
}


void demo_text_collision() {
    sdlgfx_clear();
    Uint32 start_time = SDL_GetTicks();
    MovingObject obj = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 2.0f, 1.5f, 5, 255, 0, 0, 0};
    const char* text = "DEMO DEMO DEMO";
    int text_x = SCREEN_WIDTH/2 - strlen(text) * FONT_WIDTH / 2;
    int text_y = SCREEN_HEIGHT/2 - FONT_HEIGHT / 2;

    while (running && (SDL_GetTicks() - start_time < DEMO_DURATION)) {
        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 
                                  150 + sin(SDL_GetTicks() * 0.001) * 50, 
                                  150 + cos(SDL_GetTicks() * 0.001) * 50, 
                                  150 + sin(SDL_GetTicks() * 0.002) * 50,
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

        // Обновляем позицию объекта
        update_position(&obj, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Проверяем столкновение с текстом
        int collided = 0;
        for (int i = 0; i < strlen(text); i++) {
            int char_x = text_x + i * FONT_WIDTH;
            int char_y = text_y;
            for (int px = 0; px < FONT_WIDTH; px++) {
                for (int py = 0; py < FONT_HEIGHT; py++) {
                    if (sdlgfx_is_char_pixel(text, strlen(text), i, px, 0, py)) {
                        int text_pixel_x = char_x + px;
                        int text_pixel_y = char_y + py;
                        if (abs(obj.x - text_pixel_x) < obj.size && abs(obj.y - text_pixel_y) < obj.size) {
                            collided = 1;
                            break;
                        }
                    }
                }
                if (collided) break;
            }
            if (collided) break;
        }

        // Меняем цвет объекта при столкновении
        sdlgfx_color(collided ? 0 : 255, collided ? 255 : 0, 0);
        sdlgfx_fill_circle(obj.x, obj.y, obj.size);

        // Рисуем текст
        sdlgfx_color(255, 255, 255);
        sdlgfx_string(text_x, text_y, text);

        draw_info_panel("sdlgfx_is_char_pixel", "Text Collision Detection",
                       DEMO_DURATION - (SDL_GetTicks() - start_time));
        sdlgfx_flush();
        SDL_Delay(16);
        handle_input(&running);
    }
}

void handle_input(int* running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT || 
            (e.type == SDL_KEYDOWN && 
             (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE || 
              e.key.keysym.scancode == SDL_SCANCODE_Q))) {
            *running = 0;
        }
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    sdlgfx_open(SCREEN_WIDTH, SCREEN_HEIGHT, "SDLGFX Enhanced Demo",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    sdlgfx_clear_color(0, 0, 0);
    sdlgfx_clear();


	// Загрузка стороннего шрифта. 
//	if (!sdlfont_load_psf("gr928-8x16-thin.psfu"));


    srand(time(NULL));

    while (running) {
    
	demo_points();
        if (!running) break;

        demo_triangles();
        if (!running) break;
        
        demo_circles();
        if (!running) break;
        
        demo_rectangles();
        if (!running) break;
        
        demo_lines();
        if (!running) break;
        
        demo_gradients();
        if (!running) break;
        
        demo_window_movement();
        if (!running) break;
        
        demo_pixel_effects();
        if (!running) break;
              
        demo_ellipses();
        if (!running) break;
        
        demo_arcs();
        if (!running) break;
        
        demo_text_collision();
        if (!running) break;

	demo_text();
        if (!running) break;

        sdlgfx_clear();
        sdlgfx_gradient_vertical_ex(50, 50, 50, 150, 150, 150, 
                                  0, 0, 0, 0, 0, 0, 2, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);
        sdlgfx_color(255, 255, 255);
        sdlgfx_string(SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2, "SDLGFX Demo Complete");
        sdlgfx_string(SCREEN_WIDTH/2 - 220, SCREEN_HEIGHT/2 + 30, "Press Q/Esc to exit");
        sdlgfx_flush();

        SDL_Delay(1000);
        handle_input(&running);
    }

    sdlgfx_close();
    SDL_Quit();
    return 0;
}
