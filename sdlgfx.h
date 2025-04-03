// sdlgfx.h
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
 * @file sdlgfx.h
 * @brief Header file for the simple SDL2 graphics library.
 * Original gfx library by Ivan Svarkovsky (c) 2019.
 * SDL2 port and modifications by Ivan Svarkovsky.
 */

#ifndef SDLGFX_H
#define SDLGFX_H

#include <SDL2/SDL.h>
#include <math.h>
#include "sdlfont.h" //

#ifdef __cplusplus
extern "C" {
#endif

extern SDL_Window *sdlgfx_window;  //!< The SDL window used by the library.
extern SDL_Renderer *sdlgfx_renderer; //!< The SDL renderer used by the library.
extern SDL_Texture *sdlgfx_texture; //!< The SDL texture used for rendering.

/**
 * @brief Enables or disables streaming texture usage.
 *
 * Streaming textures can be more efficient for dynamic content updates.
 *
 * @param enable Non-zero to enable streaming texture, zero to disable.
 */
void sdlgfx_set_streaming_texture(int enable);

/**
 * @brief Opens a new SDL graphics window.
 *
 * Initializes SDL video subsystem if not already initialized, creates a window and renderer,
 * and sets up the rendering texture.
 *
 * @param width The width of the window in pixels.
 * @param height The height of the window in pixels.
 * @param title The title of the window.
 * @param initial_x The initial X position of the window (SDL_WINDOWPOS_CENTERED or absolute).
 * @param initial_y The initial Y position of the window (SDL_WINDOWPOS_CENTERED or absolute).
 */
void sdlgfx_open(int width, int height, const char *title, int initial_x, int initial_y);

/**
 * @brief Closes the SDL graphics window and cleans up resources.
 *
 * Destroys the texture, renderer, and window, and quits the SDL video subsystem.
 */
void sdlgfx_close(void);

/**
 * @brief Sets the current drawing color.
 *
 * Subsequent drawing operations will use this color.
 *
 * @param r Red color component (0-255).
 * @param g Green color component (0-255).
 * @param b Blue color component (0-255).
 */
void sdlgfx_color(int r, int g, int b);

/**
 * @brief Sets the clear color for the drawing surface.
 *
 * This color is used when sdlgfx_clear() is called.
 *
 * @param r Red color component (0-255).
 * @param g Green color component (0-255).
 * @param b Blue color component (0-255).
 */
void sdlgfx_clear_color(int r, int g, int b);

/**
 * @brief Clears the drawing surface with the current clear color.
 */
void sdlgfx_clear(void);

/**
 * @brief Draws a single pixel.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 */
void sdlgfx_point(int x, int y);

/**
 * @brief Draws a line between two points.
 *
 * @param x1 The x-coordinate of the starting point.
 * @param y1 The y-coordinate of the starting point.
 * @param x2 The x-coordinate of the ending point.
 * @param y2 The y-coordinate of the ending point.
 */
void sdlgfx_line(int x1, int y1, int x2, int y2);

/**
 * @brief Draws an outlined rectangle.
 *
 * @param x1 The x-coordinate of the top-left corner.
 * @param y1 The y-coordinate of the top-left corner.
 * @param x2 The x-coordinate of the bottom-right corner.
 * @param y2 The y-coordinate of the bottom-right corner.
 */
void sdlgfx_rectangle(int x1, int y1, int x2, int y2);

/**
 * @brief Draws a filled rectangle.
 *
 * @param x1 The x-coordinate of the top-left corner.
 * @param y1 The y-coordinate of the top-left corner.
 * @param x2 The x-coordinate of the bottom-right corner.
 * @param y2 The y-coordinate of the bottom-right corner.
 */
void sdlgfx_fill_rectangle(int x1, int y1, int x2, int y2);

/**
 * @brief Draws an outlined circle.
 *
 * @param x The x-coordinate of the center.
 * @param y The y-coordinate of the center.
 * @param radius The radius of the circle.
 */
void sdlgfx_circle(int x, int y, int radius);

/**
 * @brief Draws a filled circle.
 *
 * @param x The x-coordinate of the center.
 * @param y The y-coordinate of the center.
 * @param radius The radius of the circle.
 */
void sdlgfx_fill_circle(int x, int y, int radius);

/**
 * @brief Draws an outlined triangle.
 *
 * @param x1 The x-coordinate of the first vertex.
 * @param y1 The y-coordinate of the first vertex.
 * @param x2 The x-coordinate of the second vertex.
 * @param y2 The y-coordinate of the second vertex.
 * @param x3 The x-coordinate of the third vertex.
 * @param y3 The y-coordinate of the third vertex.
 */
void sdlgfx_triangle(int x1, int y1, int x2, int y2, int x3, int y3);

/**
 * @brief Draws a filled triangle.
 *
 * @param x1 The x-coordinate of the first vertex.
 * @param y1 The y-coordinate of the first vertex.
 * @param x2 The x-coordinate of the second vertex.
 * @param y2 The y-coordinate of the second vertex.
 * @param x3 The x-coordinate of the third vertex.
 * @param y3 The y-coordinate of the third vertex.
 */
void sdlgfx_fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3);

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
                             int x3, int y3, int r3, int g3, int b3);

/**
 * @brief Draws an outlined ellipse.
 *
 * @param x The x-coordinate of the center.
 * @param y The y-coordinate of the center.
 * @param rx The horizontal radius of the ellipse.
 * @param ry The vertical radius of the ellipse.
 */
void sdlgfx_ellipse(int x, int y, int rx, int ry);

/**
 * @brief Draws a filled ellipse.
 *
 * @param x The x-coordinate of the center.
 * @param y The y-coordinate of the center.
 * @param rx The horizontal radius of the ellipse.
 * @param ry The vertical radius of the ellipse.
 */
void sdlgfx_fill_ellipse(int x, int y, int rx, int ry);

/**
 * @brief Draws an arc (part of a circle outline).
 *
 * @param x The x-coordinate of the center.
 * @param y The y-coordinate of the center.
 * @param radius The radius of the circle.
 * @param start_angle The starting angle of the arc in radians.
 * @param end_angle The ending angle of the arc in radians.
 */
void sdlgfx_arc(int x, int y, int radius, float start_angle, float end_angle);


/**
 * @brief Draws a text string using the built-in font.
 *
 * @param x The x-coordinate of the top-left corner of the text.
 * @param y The y-coordinate of the top-left corner of the text.
 * @param cc The null-terminated string to draw.
 */
void sdlgfx_string(int x, int y, const char *cc);

/**
 * @brief Flushes the renderer, making the drawn content visible.
 */
void sdlgfx_flush(void);

/**
 * @brief Waits for a key press or window close event.
 *
 * Blocks execution until a user event occurs.
 */
void sdlgfx_wait(void);

/**
 * @brief Checks if there are any pending SDL events.
 *
 * @return Non-zero if there are events waiting, zero otherwise.
 */
int sdlgfx_event_waiting(void);

/**
 * @brief Gets the width of the graphics window.
 *
 * @return The width of the window in pixels.
 */
int sdlgfx_xsize(void);

/**
 * @brief Gets the height of the graphics window.
 *
 * @return The height of the window in pixels.
 */
int sdlgfx_ysize(void);

/**
 * @brief Moves the graphics window to an absolute position.
 *
 * @param x The new x-coordinate of the window.
 * @param y The new y-coordinate of the window.
 */
void sdlgfx_move_win_abs(int x, int y);

/**
 * @brief Moves the graphics window by a relative offset.
 *
 * @param dx The horizontal offset to move the window by.
 * @param dy The vertical offset to move the window by.
 */
void sdlgfx_move_win_rel(int dx, int dy);

/**
 * @brief Retrieves the color of a pixel at the given coordinates.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @return The pixel color as an integer (0xRRGGBB).
 */
int GetPix(int x, int y);

/**
 * @brief Draws a single pixel using the current color.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 */
void sdlgfx_pixel(int x, int y);

/**
 * @brief Updates the texture with a custom drawing function.
 *
 * @param draw_func A function pointer to the drawing function.
 * @param width The width of the drawing area.
 * @param height The height of the drawing area.
 * @param time A time parameter to pass to the drawing function.
 * @param technique A technique parameter to pass to the drawing function.
 * @param update Non-zero to perform the update, zero otherwise.
 */
void sdlgfx_update_texture(void (*draw_func)(int, int, float, int), int width, int height, float time, int technique, int update);

/**
 * @brief Draws a pixel with a specified gradient color.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param r Red color component (0-255).
 * @param g Green color component (0-255).
 * @param b Blue color component (0-255).
 */
void sdlgfx_pixel_gradient(int x, int y, int r, int g, int b);

/**
 * @brief Draws a vertical color gradient.
 *
 * Supports 2, 3, or 4 color stops, and optional scanlines and noise effects.
 *
 * @param r1, g1, b1 Color for the top stop.
 * @param r2, g2, b2 Color for the second stop.
 * @param r3, g3, b3 Color for the third stop (used for 3 and 4-stop gradients).
 * @param r4, g4, b4 Color for the bottom stop (used for 4-stop gradients).
 * @param num_stops Number of color stops (2, 3, or 4).
 * @param width Width of the gradient area.
 * @param height Height of the gradient area.
 * @param scanlines_enabled Non-zero to enable scanline effect, zero otherwise.
 * @param noise_enabled Non-zero to enable line noise effect, zero otherwise.
 * @param pixel_noise_enabled Non-zero to enable pixel noise effect, zero otherwise.
 */
void sdlgfx_gradient_vertical_ex(int r1, int g1, int b1, int r2, int g2, int b2, int r3, int g3, int b3, int r4, int g4, int b4, int num_stops, int width, int height, int scanlines_enabled, int noise_enabled, int pixel_noise_enabled);

/**
 * @brief Checks if a pixel within a character bitmap is set.
 *
 * Used for advanced text effects or collision detection with text.
 *
 * @param text The text string.
 * @param text_cols The number of columns in the text layout (usually string length).
 * @param char_index_x The horizontal character index.
 * @param pixel_x_in_char The x-coordinate of the pixel within the character (0-FONT_WIDTH-1).
 * @param char_index_y The vertical character index (usually 0 for single-line text).
 * @param pixel_y_in_char The y-coordinate of the pixel within the character (0-FONT_HEIGHT-1).
 * @return SDL_TRUE if the pixel is set, SDL_FALSE otherwise.
 */
SDL_bool sdlgfx_is_char_pixel(const char* text, int text_cols, int char_index_x, int pixel_x_in_char, int char_index_y, int pixel_y_in_char);

/**
 * @brief Gets the SDL window handle used by the library.
 *
 * @return Pointer to the SDL_Window used by sdlgfx.
 */
SDL_Window* sdlgfx_get_window(void);


/* ====================================================================== */
/*                  	TEST	TEST	TEST                              */
/* ====================================================================== */

/**
 * @brief Locks the streaming texture for direct pixel access.
 *
 * @param pitch Pointer to store the pitch (in bytes) of the texture.
 * @return Pointer to the pixel data of the streaming texture, or NULL on error.
 */
void* sdlgfx_lock_texture_pixels(int *pitch);

/**
 * @brief Unlocks the streaming texture, applying pixel changes.
 */
void sdlgfx_unlock_texture_pixels(void);


#ifdef __cplusplus
}
#endif

#endif // SDLGFX_H

