
# sdlgfx - A Simple Graphics Library for SDL2

This library provides basic 2D graphics primitives for drawing in an SDL2 window.

## Table of Contents

-   [Introduction](#introduction)
-   [Compilation](#compilation)
-   [Functions](#functions)
    -   [sdlgfx\_open](#sdlgfx_open)
    -   [sdlgfx\_close](#sdlgfx_close)
    -   [sdlgfx\_color](#sdlgfx_color)
    -   [sdlgfx\_clear\_color](#sdlgfx_clear_color)
    -   [sdlgfx\_clear](#sdlgfx_clear)
    -   [sdlgfx\_point](#sdlgfx_point)
    -   [sdlgfx\_line](#sdlgfx_line)
    -   [sdlgfx\_rectangle](#sdlgfx_rectangle)
    -   [sdlgfx\_fill\_rectangle](#sdlgfx_fill_rectangle)
    -   [sdlgfx\_circle](#sdlgfx_circle)
    -   [sdlgfx\_fill\_circle](#sdlgfx_fill_circle)
    -   [sdlgfx\_string](#sdlgfx_string)
    -   [sdlgfx\_flush](#sdlgfx_flush)
    -   [sdlgfx\_wait](#sdlgfx_wait)
    -   [sdlgfx\_event\_waiting](#sdlgfx_event_waiting)
    -   [sdlgfx\_xsize](#sdlgfx_xsize)
    -   [sdlgfx\_ysize](#sdlgfx_ysize)
    -   [sdlgfx\_move\_win\_abs](#sdlgfx_move_win_abs)
    -   [sdlgfx\_move\_win\_rel](#sdlgfx_move_win_rel)
    -   [GetPix](#getpix)
-   [Dependencies](#dependencies)
-   [License](#license)

## Introduction

`sdlgfx` is a lightweight graphics library designed to simplify 2D drawing operations in SDL2 applications. It offers a set of functions for creating and managing a window, setting colors, clearing the screen, and drawing basic shapes and text.

## Compilation

To compile a program that uses `sdlgfx`, you'll need to include the `sdlgfx.c`, `sdlfont.c` files and link against the SDL2 library. Example:

```bash
gcc -o test_sdlgfx test_sdlgfx.c sdlgfx.c sdlfont.c -lSDL2 -lm
```

## Functions

### sdlgfx\_open

```c
void sdlgfx_open(int width, int height, const char *title, int initial_x, int initial_y);
```

Initializes the graphics window. This function creates an SDL2 window and renderer, setting up the environment for drawing.

*   **Parameters:**
    *   `width`: The width of the window in pixels.
    *   `height`: The height of the window in pixels.
    *   `title`: The title of the window.
    *   `initial_x`: The initial X coordinate of the window's position on the screen. Use `SDL_WINDOWPOS_UNDEFINED` or `SDL_WINDOWPOS_CENTERED` for default positioning.
    *   `initial_y`: The initial Y coordinate of the window's position on the screen. Use `SDL_WINDOWPOS_UNDEFINED` or `SDL_WINDOWPOS_CENTERED` for default positioning.
*   **Example:**

```c
sdlgfx_open(640, 480, "My SDLGFX Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
```

This will create a 640x480 window with the title "My SDLGFX Window", centered on the screen.

### sdlgfx\_close

```c
void sdlgfx_close(void);
```

Closes the graphics window and releases all resources.  It destroys the SDL renderer and window, and quits the SDL library.

*   **Parameters:** None
*   **Example:**

```c
sdlgfx_close();
```

### sdlgfx\_color

```c
void sdlgfx_color(int r, int g, int b);
```

Sets the current drawing color.  This function defines the color used for subsequent drawing operations. The alpha component is implicitly set to 255 (fully opaque).

*   **Parameters:**
    *   `r`: The red component of the color (0-255).
    *   `g`: The green component of the color (0-255).
    *   `b`: The blue component of the color (0-255).
*   **Example:**

```c
sdlgfx_color(255, 0, 0); // Set the color to red
```

### sdlgfx\_clear\_color

```c
void sdlgfx_clear_color(int r, int g, int b);
```

Sets the default color used for clearing the screen.  This function defines the color used by `sdlgfx_clear()`.

*   **Parameters:**
    *   `r`: The red component of the color (0-255).
    *   `g`: The green component of the color (0-255).
    *   `b`: The blue component of the color (0-255).
*   **Example:**

```c
sdlgfx_clear_color(0, 0, 255); // Set the clear color to blue
```

### sdlgfx\_clear

```c
void sdlgfx_clear(void);
```

Clears the screen using the currently set clear color.  This function fills the entire window with the color defined by `sdlgfx_clear_color()`.

*   **Parameters:** None
*   **Example:**

```c
sdlgfx_clear();
```

### sdlgfx\_point

```c
void sdlgfx_point(int x, int y);
```

Draws a point at the specified coordinates.

*   **Parameters:**
    *   `x`: The x-coordinate of the point.
    *   `y`: The y-coordinate of the point.
*   **Example:**

```c
sdlgfx_color(255, 255, 255); // Set color to white
sdlgfx_point(100, 150);       // Draw a white pixel at (100, 150)
```

### sdlgfx\_line

```c
void sdlgfx_line(int x1, int y1, int x2, int y2);
```

Draws a line between two points.

*   **Parameters:**
    *   `x1`: The x-coordinate of the starting point.
    *   `y1`: The y-coordinate of the starting point.
    *   `x2`: The x-coordinate of the ending point.
    *   `y2`: The y-coordinate of the ending point.
*   **Example:**

```c
sdlgfx_color(0, 255, 0);   // Set color to green
sdlgfx_line(50, 50, 200, 100); // Draw a green line from (50, 50) to (200, 100)
```

### sdlgfx\_rectangle

```c
void sdlgfx_rectangle(int x1, int y1, int x2, int y2);
```

Draws a rectangle outline.  The coordinates specify the top-left and bottom-right corners of the rectangle.

*   **Parameters:**
    *   `x1`: The x-coordinate of the top-left corner.
    *   `y1`: The y-coordinate of the top-left corner.
    *   `x2`: The x-coordinate of the bottom-right corner.
    *   `y2`: The y-coordinate of the bottom-right corner.
*   **Example:**

```c
sdlgfx_color(0, 0, 255);     // Set color to blue
sdlgfx_rectangle(10, 10, 100, 50); // Draw a blue rectangle outline
```

### sdlgfx\_fill\_rectangle

```c
void sdlgfx_fill_rectangle(int x1, int y1, int x2, int y2);
```

Draws a filled rectangle. The coordinates specify the top-left and bottom-right corners of the rectangle.

*   **Parameters:**
    *   `x1`: The x-coordinate of the top-left corner.
    *   `y1`: The y-coordinate of the top-left corner.
    *   `x2`: The x-coordinate of the bottom-right corner.
    *   `y2`: The y-coordinate of the bottom-right corner.
*   **Example:**

```c
sdlgfx_color(255, 255, 0);         // Set color to yellow
sdlgfx_fill_rectangle(120, 60, 220, 110); // Draw a filled yellow rectangle
```

### sdlgfx\_circle

```c
void sdlgfx_circle(int x, int y, int radius);
```

Draws a circle outline. Uses a line-based approximation.

*   **Parameters:**
    *   `x`: The x-coordinate of the circle's center.
    *   `y`: The y-coordinate of the circle's center.
    *   `radius`: The radius of the circle.
*   **Example:**

```c
sdlgfx_color(255, 0, 255);   // Set color to magenta
sdlgfx_circle(320, 240, 50);   // Draw a magenta circle
```

### sdlgfx\_fill\_circle

```c
void sdlgfx_fill_circle(int x, int y, int radius);
```

Draws a filled circle. Uses a line-based approximation.

*   **Parameters:**
    *   `x`: The x-coordinate of the circle's center.
    *   `y`: The y-coordinate of the circle's center.
    *   `radius`: The radius of the circle.
*   **Example:**

```c
sdlgfx_color(0, 255, 255);      // Set color to cyan
sdlgfx_fill_circle(400, 300, 30);  // Draw a cyan filled circle
```

### sdlgfx\_string

```c
void sdlgfx_string(int x, int y, const char *cc);
```

Draws a string of text at the specified coordinates.  Uses the `sdlfont` library.

*   **Parameters:**
    *   `x`: The x-coordinate of the text's top-left corner.
    *   `y`: The y-coordinate of the text's top-left corner.
    *   `cc`: The string to draw.
*   **Example:**

```c
sdlgfx_color(255, 255, 255); // Set color to white
sdlgfx_string(10, 400, "Hello, SDLGFX!"); // Draw "Hello, SDLGFX!"
```

### sdlgfx\_flush

```c
void sdlgfx_flush(void);
```

Presents the back buffer to the screen, displaying all drawing operations performed since the last flush.

*   **Parameters:** None
*   **Example:**

```c
sdlgfx_clear();
sdlgfx_color(255, 255, 255);
sdlgfx_string(10, 10, "Drawing...");
sdlgfx_flush(); // Update the screen
```

### sdlgfx\_wait

```c
void sdlgfx_wait(void);
```

Waits for a key press or the window to be closed. This function pauses the program until a key is pressed or the window is closed.  Useful for keeping the window open until the user is ready to close it.

*   **Parameters:** None
*   **Example:**

```c
sdlgfx_clear();
sdlgfx_color(255, 255, 255);
sdlgfx_string(10, 10, "Press any key to exit...");
sdlgfx_flush();
sdlgfx_wait(); // Wait for a key press or window close
```

### sdlgfx\_event\_waiting

```c
int sdlgfx_event_waiting(void);
```

Checks if there are any pending events in the SDL event queue.

*   **Parameters:** None
*   **Returns:** Non-zero if there are events waiting, 0 otherwise.
*   **Example:**

```c
if (sdlgfx_event_waiting()) {
    // Handle events
}
```

### sdlgfx\_xsize

```c
int sdlgfx_xsize(void);
```

Returns the width of the graphics window.

*   **Parameters:** None
*   **Returns:** The width of the window in pixels.
*   **Example:**

```c
int width = sdlgfx_xsize();
printf("Window width: %d\n", width);
```

### sdlgfx\_ysize

```c
int sdlgfx_ysize(void);
```

Returns the height of the graphics window.

*   **Parameters:** None
*   **Returns:** The height of the window in pixels.
*   **Example:**

```c
int height = sdlgfx_ysize();
printf("Window height: %d\n", height);
```

### sdlgfx\_move\_win\_abs

```c
void sdlgfx_move_win_abs(int x, int y);
```

Moves the graphics window to the specified absolute screen coordinates.

*   **Parameters:**
    *   `x`: The new X coordinate of the window's top-left corner.
    *   `y`: The new Y coordinate of the window's top-left corner.
*   **Example:**

```c
sdlgfx_move_win_abs(100, 200); // Move the window to (100, 200)
```

### sdlgfx\_move\_win\_rel

```c
void sdlgfx_move_win_rel(int dx, int dy);
```

Moves the graphics window by the specified relative offset.

*   **Parameters:**
    *   `dx`: The horizontal offset.
    *   `dy`: The vertical offset.
*   **Example:**

```c
sdlgfx_move_win_rel(10, -20); // Move the window 10 pixels right and 20 pixels up
```

### GetPix

```c
int GetPix(int x, int y);
```

Gets the color of the pixel at the specified coordinates.  This is a relatively slow operation as it involves reading pixels from the renderer.

*   **Parameters:**
    *   `x`: The x-coordinate of the pixel.
    *   `y`: The y-coordinate of the pixel.
*   **Returns:** The color of the pixel as an integer in the format `0xRRGGBB`. Returns 0 if the coordinates are out of bounds or if there's an error.
*   **Example:**

```c
int color = GetPix(50, 50);
int red = (color >> 16) & 0xFF;
int green = (color >> 8) & 0xFF;
int blue = color & 0xFF;
printf("Pixel color at (50, 50): R=%d, G=%d, B=%d\n", red, green, blue);
```

## Dependencies

*   SDL2
*   sdlfont (Included alongside sdlgfx)
*   math.h (for circle functions)

## License

This library is licensed under the MIT License. See the `LICENSE` file for more information.
