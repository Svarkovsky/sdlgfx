/*

		-	sdlgfx_demonstrations	-

*/


/*

gcc -std=c99 -o sdlgfx_demonstrations sdlgfx_demonstrations.c sdlgfx.c sdlfont.c -lSDL2 -lm -O3 -march=native -mtune=native

Or like this, to be more specific:
gcc -std=c99 -o sdlgfx_demonstrations sdlgfx_demonstrations.c sdlgfx.c sdlfont.c -lSDL2 -lm -O3 -march=native -mtune=native -msse3 -mssse3 -fno-exceptions -fomit-frame-pointer -flto -fvisibility=hidden -mfpmath=sse -ffast-math -pipe -s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wl,--strip-all -DNDEBUG

*/

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

#include <SDL2/SDL.h> // Подключаем заголовочный файл библиотеки SDL2 для работы с графикой и окном
#include <math.h>    // Подключаем заголовочный файл math.h для математических функций, таких как sin(), cos(), и т.д.

#ifndef M_PI // Проверяем, не определена ли уже константа M_PI
#define M_PI acos(-1.0) // Если M_PI не определена, определяем её как арккосинус от -1.0 (что равно Пи)
#endif

#include "sdlgfx.h"   // Подключаем наш собственный заголовочный файл "sdlgfx.h", вероятно, содержащий функции для работы с графикой
#include <string.h>  // Подключаем заголовочный файл string.h для работы со строками, например, для функции strcmp()
#include <stdio.h>   // Подключаем заголовочный файл stdio.h для стандартного ввода/вывода, например, для функции printf() и sprintf()

#define COPYRIGHT_TEXT "Copyright (c) Ivan Svarkovsky - 2025" // Определяем макрос COPYRIGHT_TEXT для хранения текста авторских прав

/* Глобальное определение структуры для цветовой палитры */
struct Color {
    int r, g, b;       // Целочисленные переменные для хранения компонент цвета: красный (r), зеленый (g), синий (b)
    const char *name;  // Указатель на константную строку, хранящую имя цвета (например, "Red", "Green")
};

/* Функция для рисования пунктирной линии */
void sdlgfx_dashed_line(int x1, int y1, int x2, int y2, int dash_length, int gap_length)
{
    // x1, y1 - координаты начала линии
    // x2, y2 - координаты конца линии
    // dash_length - длина штриха пунктира
    // gap_length - длина пробела между штрихами пунктира

    int dx = abs(x2 - x1); // Вычисляем абсолютное значение разности x-координат (длина линии по горизонтали)
    int dy = abs(y2 - y1); // Вычисляем абсолютное значение разности y-координат (длина линии по вертикали)
    int sx = (x1 < x2) ? 1 : -1; // Определяем направление шага по x (1 - вправо, -1 - влево)
    int sy = (y1 < y2) ? 1 : -1; // Определяем направление шага по y (1 - вниз, -1 - вверх)
    int err = dx - dy;         // Инициализируем переменную ошибки для алгоритма Брезенхэма
    int x = x1, y = y1;       // Инициализируем текущие координаты точки начала линии
    int dash_counter = 0;      // Счетчик для отслеживания длины текущего штриха или пробела
    int draw_dash = 1;         // Флаг, определяющий, нужно ли рисовать штрих (1 - рисовать, 0 - пробел)

    while (1) { // Бесконечный цикл, который прерывается, когда достигается конец линии
        if (draw_dash) // Если флаг draw_dash установлен (нужно рисовать штрих)
            sdlgfx_point(x, y); // Рисуем точку в текущих координатах (x, y) - функция из sdlgfx.h
        dash_counter++; // Увеличиваем счетчик штриха/пробела

        if (draw_dash) { // Если сейчас рисуется штрих
            if (dash_counter >= dash_length) { // Если счетчик достиг длины штриха
                draw_dash = 0;       // Переключаемся в режим "пробел" - не рисовать
                dash_counter = 0;      // Сбрасываем счетчик
            }
        } else { // Если сейчас пробел
            if (dash_counter >= gap_length) { // Если счетчик достиг длины пробела
                draw_dash = 1;       // Переключаемся в режим "рисовать штрих"
                dash_counter = 0;      // Сбрасываем счетчик
            }
        }

        if (x == x2 && y == y2) break; // Если достигнуты координаты конца линии, выходим из цикла
        int e2 = 2 * err;                // Вычисляем 2*err для оптимизации сравнений
        if (e2 > -dy) {                  // Условие для шага по x
            err -= dy;                   // Обновляем ошибку
            x += sx;                     // Делаем шаг по x в нужном направлении
        }
        if (e2 < dx) {                   // Условие для шага по y
            err += dx;                   // Обновляем ошибку
            y += sy;                     // Делаем шаг по y в нужном направлении
        }
    }
}

/* Функция для генерации радужного цвета на основе фазы */
void get_rainbow_color(float t, int *r, int *g, int *b)
{
    // t - фаза, значение от 0 до 2*PI, определяющее положение цвета в радуге
    // *r, *g, *b - указатели на целочисленные переменные, куда будут записаны компоненты RGB цвета

    *r = (int)(sin(t) * 127 + 128);                // Вычисляем красную компоненту цвета на основе синуса фазы t
    *g = (int)(sin(t + 2 * M_PI / 3) * 127 + 128); // Вычисляем зеленую компоненту цвета, сдвигая фазу на 2*PI/3
    *b = (int)(sin(t + 4 * M_PI / 3) * 127 + 128); // Вычисляем синюю компоненту цвета, сдвигая фазу на 4*PI/3
    // Формула (sin(t) * 127 + 128) преобразует синус от -1 до 1 в диапазон от 0 до 255
    // Сдвиги фазы для G и B компонент создают эффект радуги
}

/* Структура для параметров фигуры Лиссажу */
typedef struct {
    float a;          // Частота по оси X
    float b;          // Частота по оси Y
    const char *name; // Имя фигуры (для отображения)
} LissajousFigure;

// Массив структур LissajousFigure, определяющий параметры различных фигур Лиссажу
LissajousFigure figures[] = {
    {1.0, 1.0, "1:1 Circle/Line"},     // Круг или линия
    {1.0, 2.0, "1:2 Parabola"},        // Парабола
    {2.0, 1.0, "2:1 Vertical Parabola"}, // Вертикальная парабола
    {3.0, 2.0, "3:2 Lissajous"},       // Фигура Лиссажу 3:2
    {3.0, 4.0, "3:4 Lissajous"},       // Фигура Лиссажу 3:4
    {4.0, 3.0, "4:3 Lissajous"},       // Фигура Лиссажу 4:3
    {5.0, 4.0, "5:4 Lissajous"},       // Фигура Лиссажу 5:4
    {5.0, 6.0, "5:6 Lissajous"},       // Фигура Лиссажу 5:6
    {1.0, 3.0, "1:3 Lissajous"},       // Фигура Лиссажу 1:3
    {3.0, 1.0, "3:1 Lissajous"},       // Фигура Лиссажу 3:1
    {2.0, 2.0, "2:2 Square-like"},     // Квадратоподобная фигура
    {5.0, 3.0, "5:3 Lissajous"},       // Фигура Лиссажу 5:3
    {7.0, 4.0, "7:4 Lissajous"},       // Фигура Лиссажу 7:4
    {7.0, 6.0, "7:6 Lissajous"},       // Фигура Лиссажу 7:6
    {8.0, 5.0, "8:5 Lissajous"},       // Фигура Лиссажу 8:5
    {9.0, 7.0, "9:7 Lissajous"},       // Фигура Лиссажу 9:7
    {10.0, 7.0, "10:7 Lissajous"},     // Фигура Лиссажу 10:7
    {10.0, 9.0, "10:9 Lissajous"},     // Фигура Лиссажу 10:9
    {11.0, 8.0, "11:8 Lissajous"},     // Фигура Лиссажу 11:8
    {11.0, 9.0, "11:9 Lissajous"},     // Фигура Лиссажу 11:9
    {12.0, 7.0, "12:7 Lissajous"},     // Фигура Лиссажу 12:7
    {13.0, 5.0, "13:5 Lissajous"}      // Фигура Лиссажу 13:5
};
int num_figures = sizeof(figures) / sizeof(figures[0]); // Вычисляем количество фигур в массиве figures
int current_figure_index = 0;                            // Индекс текущей отображаемой фигуры (начинаем с первой фигуры)

float scale = 1.0;          /* Масштабный коэффициент для фигуры */
int x_offset = 0;           /* Смещение фигуры по оси X для перемещения */
int y_offset = 0;           /* Смещение фигуры по оси Y для перемещения */
float phase_increment = 0.05; /* Шаг приращения фазы для анимации (скорость анимации) */
unsigned int figure_change_interval_ms = 10000; /* Интервал смены фигуры в миллисекундах (10 секунд) */
unsigned int last_figure_change_time;           // Время последнего изменения фигуры
int countdown_seconds = 0;                     // Счетчик секунд до автоматической смены фигуры
int show_countdown = 0;                         /* Флаг для показа обратного отсчета перед сменой фигуры (0 = выкл, 1 = вкл) */

/* Функция для рисования фигуры Лиссажу */
void draw_lissajous(int width, int height, float phase, LissajousFigure fig)
{
    // width, height - ширина и высота области рисования
    // phase - фаза анимации
    // fig - структура LissajousFigure, определяющая параметры фигуры

    const int num_points = 5000; // Увеличиваем количество точек для более гладкой кривой
    float a = fig.a;             // Частота X из структуры
    float b = fig.b;             // Частота Y из структуры
    float delta = phase;         // Текущая фаза
    int thickness = 1;           // Толщина линии

    float figure_scale = 0.95;   // Небольшое уменьшение масштаба фигуры, чтобы поместить ее в рамку

    for (int i = 0; i < num_points; i++) { // Цикл по точкам, составляющим фигуру
        float t = (float)i / (num_points - 1) * 2 * M_PI; // Параметрическое значение t от 0 до 2*PI
        int x = width / 2 + x_offset + (int)((width / 6 * sin(a * t + delta)) * scale * figure_scale); // Вычисление x-координаты точки фигуры
        int y = height / 2 + y_offset + (int)((height / 6 * sin(b * t)) * scale * figure_scale); // Вычисление y-координаты точки фигуры
        // width / 2, height / 2 - центрирование фигуры в окне
        // x_offset, y_offset - смещение фигуры
        // scale - масштаб фигуры
        // figure_scale - дополнительное уменьшение масштаба
        // sin(a * t + delta), sin(b * t) - функции, формирующие фигуру Лиссажу
        int r, g, b;
        get_rainbow_color(t + phase, &r, &g, &b); // Получаем радужный цвет для текущей точки
        sdlgfx_color(r, g, b);                    // Устанавливаем цвет рисования

        for (int dx = -thickness; dx <= thickness; dx++) // Цикл для увеличения толщины линии (рисуем несколько точек вокруг основной точки)
            for (int dy = -thickness; dy <= thickness; dy++)
                sdlgfx_point(x + dx, y + dy); // Рисуем точку с заданным цветом - функция из sdlgfx.h
    }
}

/* Функция для рисования точечной рамки */
void draw_point_border(int x1, int y1, int x2, int y2) {
    // x1, y1 - координаты левого верхнего угла рамки
    // x2, y2 - координаты правого нижнего угла рамки

    int color1_r = 100, color1_g = 100, color1_b = 100; // Светлый серый цвет для верхней и левой сторон рамки
    int color2_r = 50, color2_g = 50, color2_b = 50;   // Темный серый цвет для нижней и правой сторон рамки
    int corner_r = 255, corner_g = 255, corner_b = 0; // **Желтый цвет для углов рамки**

    // Верхняя и левая границы - рисуем светлым серым цветом (color1)
    sdlgfx_color(color1_r, color1_g, color1_b);    // Устанавливаем цвет рисования
    for (int x = x1; x <= x2; x += 3) sdlgfx_point(x, y1); // Рисуем точки для верхней границы (каждые 3 пикселя по горизонтали)
    for (int y = y1; y <= y2; y += 3) sdlgfx_point(x1, y); // Рисуем точки для левой границы (каждые 3 пикселя по вертикали)

    // Нижняя и правая границы - рисуем темным серым цветом (color2)
    sdlgfx_color(color2_r, color2_g, color2_b);    // Устанавливаем цвет рисования
    for (int x = x1; x <= x2; x += 3) sdlgfx_point(x, y2); // Рисуем точки для нижней границы (каждые 3 пикселя по горизонтали)
    for (int y = y1; y <= y2; y += 3) sdlgfx_point(x2, y); // Рисуем точки для правой границы (каждые 3 пикселя по вертикали)

    // **Углы - теперь рисуем желтым цветом (corner_color), чтобы выделить их**
    sdlgfx_color(corner_r, corner_g, corner_b);    // Устанавливаем цвет рисования - желтый
    sdlgfx_point(x1, y1); // Левый верхний угол
    sdlgfx_point(x2, y1); // Правый верхний угол
    sdlgfx_point(x1, y2); // Левый нижний угол
    sdlgfx_point(x2, y2); // Правый нижний угол
}


/* Главная функция программы */
int main(int argc, char *argv[])
{
    // argc - количество аргументов командной строки
    // argv - массив строк, содержащий аргументы командной строки

    if (SDL_Init(SDL_INIT_VIDEO) < 0) { // Инициализируем подсистему видео SDL
        SDL_Log("SDL initialization failed: %s", SDL_GetError()); // Если инициализация не удалась, выводим сообщение об ошибке в лог SDL
        return 1;                                                // Возвращаем код ошибки 1, указывающий на неудачное завершение программы
    }

    sdlgfx_open(800, 600, "SDLGFX v0.1 - simple graphics library - Demo", // Открываем окно SDL с заданными параметрами - функция из sdlgfx.h
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    // 800, 600 - ширина и высота окна
    // "SDLGFX v0.1 - simple graphics library - Demo" - заголовок окна
    // SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED - позиционирование окна по центру экрана

    sdlgfx_clear_color(0, 0, 0); // Устанавливаем цвет фона для очистки экрана - черный цвет (RGB: 0, 0, 0) - функция из sdlgfx.h
    sdlgfx_clear();             // Очищаем экран, заливая его фоновым цветом - функция из sdlgfx.h

    /* Определяем размеры рамки */
    int border_x1 = 200;      // x-координата левой границы рамки
    int border_y1 = 150;      // y-координата верхней границы рамки
    int border_x2 = 600;      // x-координата правой границы рамки
    int border_y2 = 500;      // y-координата нижней границы рамки

    /* Вычисляем центр рамки */
    int border_center_x = (border_x1 + border_x2) / 2; // Вычисляем x-координату центра рамки
    int border_center_y = (border_y1 + border_y2) / 2; // Вычисляем y-координату центра рамки

    /* Вычисляем максимальные смещения с запасом в 2 пикселя от границы рамки */
    int max_x_offset = (border_x2 - border_x1) / 2 - 2; // Максимальное смещение по x от центра рамки (половина ширины рамки минус 2 пикселя)
    int max_y_offset = (border_y2 - border_y1) / 2 - 2; // Максимальное смещение по y от центра рамки (половина высоты рамки минус 2 пикселя)

    /* Определяем центр экрана */
    int width = 800;         // Ширина экрана
    int height = 600;        // Высота экрана
    int screen_center_x = width / 2;  // x-координата центра экрана
    int screen_center_y = height / 2;  // y-координата центра экрана

    /* Корректируем лимиты смещения на основе разницы между центром экрана и центром рамки */
    int x_offset_limit_left = -(max_x_offset - (screen_center_x - border_center_x));   // Левый предел смещения по x
    int x_offset_limit_right = max_x_offset + (screen_center_x - border_center_x);  // Правый предел смещения по x
    int y_offset_limit_up = -(max_y_offset - (screen_center_y - border_center_y));     // Верхний предел смещения по y
    int y_offset_limit_down = max_y_offset + (screen_center_y - border_center_y);    // Нижний предел смещения по y
    // Эти лимиты нужны, чтобы фигура не выходила за рамку при смещении, даже если центр рамки не совпадает с центром экрана

    struct Color palette[] = { // Объявляем и инициализируем массив цветовой палитры
        {255, 0, 0, "Red"},        // Красный
        {0, 255, 0, "Green"},      // Зеленый
        {0, 0, 255, "Blue"},       // Синий
        {255, 255, 0, "Yellow"},    // Желтый
        {255, 0, 255, "Magenta"},   // Маджента
        {0, 255, 255, "Cyan"},      // Циан
        {255, 255, 255, "White"},     // Белый
        {0, 0, 0, "Black"}         // Черный
    };
    int palette_size = sizeof(palette) / sizeof(palette[0]); // Вычисляем размер палитры

    int rect_width = 50, rect_height = 50, start_x = 50, start_y = 50; // Размеры прямоугольников палитры и начальные координаты
    for (int i = 0; i < palette_size; i++) { // Цикл по всем цветам в палитре
        int x = start_x + (i * (rect_width + 10)); // Вычисляем x-координату для текущего прямоугольника палитры
        int y = start_y;                            // y-координата для всех прямоугольников одинакова
        sdlgfx_color(palette[i].r, palette[i].g, palette[i].b); // Устанавливаем цвет рисования из текущего цвета палитры
        sdlgfx_fill_rectangle(x, y, x + rect_width, y + rect_height); // Рисуем залитый прямоугольник - функция из sdlgfx.h
        sdlgfx_color(255, 255, 255);                                 // Устанавливаем цвет рисования - белый для текста
        sdlgfx_string(x, y + rect_height + 10, palette[i].name);    // Выводим имя цвета под прямоугольником - функция из sdlgfx.h

        if (strcmp(palette[i].name, "Black") == 0) { // Если текущий цвет - черный
            sdlgfx_color(255, 255, 255);             // Устанавливаем цвет рисования - белый для контура
            sdlgfx_dashed_line(x, y, x + rect_width, y, 5, 5);           // Рисуем пунктирную верхнюю линию контура
            sdlgfx_dashed_line(x + rect_width, y, x + rect_width, y + rect_height, 5, 5); // Рисуем пунктирную правую линию контура
            sdlgfx_dashed_line(x + rect_width, y + rect_height, x, y + rect_height, 5, 5); // Рисуем пунктирную нижнюю линию контура
            sdlgfx_dashed_line(x, y + rect_height, x, y, 5, 5);           // Рисуем пунктирную левую линию контура
            // Пунктирный контур для черного цвета, чтобы он был виден на черном фоне
        }
    }

    /* Рисуем статические текстовые метки (сдвинуты вниз на 20 пикселей) */
    sdlgfx_color(255, 255, 255); // Устанавливаем цвет рисования - белый для текста
    sdlgfx_string(50, 520, "Press 'q' to quit, arrows to move, +/- to scale, 1-9/0/N/P to change figure"); // Выводим строку инструкций
    sdlgfx_string(50, 540, "Hover over colors to get RGB values");                                  // Выводим строку подсказки про RGB

    int black_check = GetPix(0, 0); // Получаем цвет пикселя в координатах (0, 0) - функция из sdlgfx.h
    printf("Black check at (0, 0): %d\n", black_check); // Выводим значение цвета пикселя (для отладки, проверка черного фона)

    float phase = 0.0;          // Начальная фаза анимации
    int running = 1;            // Флаг, определяющий, работает ли программа (1 - работает, 0 - завершить)
    int mouse_x = 0, mouse_y = 0; // Координаты мыши
    last_figure_change_time = SDL_GetTicks(); // Запоминаем время старта программы для отсчета времени смены фигуры

	/* COPYRIGHT */
	sdlgfx_color(255, 255, 255); // Устанавливаем цвет для текста авторских прав
	sdlgfx_string(500, 10, COPYRIGHT_TEXT); // Выводим текст авторских прав в правом верхнем углу

    while (running) { // Главный цикл программы
        SDL_Event e;    // Переменная для хранения событий SDL
        while (SDL_PollEvent(&e)) { // Обрабатываем очередь событий SDL
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_Q)) 
                                                                            // Если событие - закрытие окна или нажатие клавиши 'Q'
                running = 0; // Устанавливаем флаг running в 0, чтобы завершить программу
            if (e.type == SDL_MOUSEMOTION) { // Если событие - движение мыши
                mouse_x = e.motion.x;    // Запоминаем x-координату мыши
                mouse_y = e.motion.y;    // Запоминаем y-координату мыши
            }
            if (e.type == SDL_KEYDOWN) { // Если событие - нажатие клавиши
                show_countdown = 0; // Отключаем показ countdown при ручном управлении
                last_figure_change_time = SDL_GetTicks(); // Сбрасываем таймер автосмены фигуры при ручном управлении
                switch (e.key.keysym.scancode) {
                    case SDL_SCANCODE_LEFT: // Если нажата клавиша "влево"
                        x_offset -= 10; // Уменьшаем смещение по оси X
                        if (x_offset < -50) x_offset = -50; // Ограничиваем смещение (не даем выйти за границу)
                        break;
                    case SDL_SCANCODE_RIGHT: // Если нажата клавиша "вправо"
                        x_offset += 10; // Увеличиваем смещение по оси X
                        if (x_offset > 50) x_offset = 50; // Ограничиваем смещение (не даем выйти за границу)
                        break;
                    case SDL_SCANCODE_UP: // Если нажата клавиша "вверх"
                        y_offset -= 10; // Уменьшаем смещение по оси Y
                        if (y_offset < -37) y_offset = -37; // Ограничиваем смещение (не даем выйти за границу)
                        break;
                    case SDL_SCANCODE_DOWN: // Если нажата клавиша "вниз"
                        y_offset += 10; // Увеличиваем смещение по оси Y
                        if (y_offset > 87) y_offset = 87; // Ограничиваем смещение (не даем выйти за границу)
                        break;
                    case SDL_SCANCODE_KP_PLUS: // Клавиша "+" на цифровой клавиатуре
                    case SDL_SCANCODE_EQUALS:  // Клавиша "="
                        scale *= 1.1f;         // Увеличиваем масштаб фигуры
                        if (scale > 1.15f) scale = 1.15f; // Ограничиваем максимальный масштаб
                        break;
                    case SDL_SCANCODE_MINUS: // Клавиша "-"
                        scale /= 1.1f;         // Уменьшаем масштаб фигуры
                        if (scale < 0.75f) scale = 0.75f;  // Ограничиваем минимальный масштаб
                        break;
                    case SDL_SCANCODE_1: current_figure_index = 0; break; // Клавиша "1" - выбираем первую фигуру
                    case SDL_SCANCODE_2: current_figure_index = 1; break; // Клавиша "2" - выбираем вторую фигуру
                    case SDL_SCANCODE_3: current_figure_index = 2; break; // ... и так далее до клавиши "0"
                    case SDL_SCANCODE_4: current_figure_index = 3; break;
                    case SDL_SCANCODE_5: current_figure_index = 4; break;
                    case SDL_SCANCODE_6: current_figure_index = 5; break;
                    case SDL_SCANCODE_7: current_figure_index = 6; break;
                    case SDL_SCANCODE_8: current_figure_index = 7; break;
                    case SDL_SCANCODE_9: current_figure_index = 8; break;
                    case SDL_SCANCODE_0: current_figure_index = 9; break; // Клавиша "0" - выбираем десятую фигуру (индекс 9)
                    case SDL_SCANCODE_N: // Клавиша "N" - следующая фигура
                        current_figure_index++; // Переключаемся к следующей фигуре
                        show_countdown = 1;    // Включаем показ countdown при автосмене
                        break;
                    case SDL_SCANCODE_P: // Клавиша "P" - предыдущая фигура
                        current_figure_index--; // Переключаемся к предыдущей фигуре
                        show_countdown = 1;    // Включаем показ countdown при автосмене
                        break;
                }
                if (current_figure_index >= num_figures) current_figure_index = num_figures - 1; 
                     // Предотвращаем выход индекса за границы массива фигур (если индекс стал больше или равен количеству фигур)
                if (current_figure_index < 0) current_figure_index = 0; 
                    // Предотвращаем выход индекса за границы массива фигур (если индекс стал меньше 0)
            }
        }

        sdlgfx_color(0, 0, 0); // Устанавливаем цвет рисования - черный для очистки области фигуры
        sdlgfx_fill_rectangle(202, 152, 598, 498); // Заполняем прямоугольник черным цветом, очищая предыдущую фигуру 
                                                  // (немного меньше рамки, чтобы рамка оставалась видна)

        draw_point_border(border_x1, border_y1, border_x2, border_y2); // Рисуем точечную рамку - функция draw_point_border

        draw_lissajous(800, 600, phase, figures[current_figure_index]); // Рисуем фигуру Лиссажу - функция draw_lissajous
        phase += phase_increment; // Увеличиваем фазу для анимации

        sdlgfx_color(0, 0, 0); // Устанавливаем цвет рисования - черный для очистки области RGB текста
        sdlgfx_fill_rectangle(60, 20, 300, 30); /* Очищаем область для RGB текста */
        sdlgfx_fill_rectangle(50, 560, 750, 580); /* Очищаем область информации о фигуре (сдвинутую вниз) */
        sdlgfx_fill_rectangle(50, 580, 750, 600); /* Очищаем область countdown (сдвинутую вниз) */

        for (int i = 0; i < palette_size; i++) { // Цикл по цветам палитры для проверки наведения мыши
            int x = start_x + (i * (rect_width + 10)); // Вычисляем x-координату прямоугольника палитры
            int y = start_y;                            // y-координата прямоугольника палитры
            if (mouse_x >= x && mouse_x < x + rect_width && mouse_y >= y && mouse_y < y + rect_height) { 
                                  // Проверяем, находится ли мышь в пределах текущего прямоугольника палитры
                int pixel_color = GetPix(mouse_x, mouse_y); // Получаем цвет пикселя под мышью - функция GetPix из sdlgfx.h
                int r = (pixel_color >> 16) & 0xFF;       // Извлекаем красную компоненту цвета из целого числа
                int g = (pixel_color >> 8) & 0xFF;        // Извлекаем зеленую компоненту цвета
                int b = (pixel_color & 0xFF);             // Извлекаем синюю компоненту цвета
                printf("GetPix(%d, %d) = %d, RGB: (%d, %d, %d)\n", mouse_x, mouse_y, pixel_color, r, g, b); 
                                 // Выводим RGB значения в консоль (для отладки)
                char rgb_text[50];                                                                  // Объявляем буфер для строки RGB
                snprintf(rgb_text, sizeof(rgb_text), "RGB: (%d, %d, %d)", r, g, b);              // Форматируем строку RGB
                sdlgfx_color(255, 255, 255);                                                    // Устанавливаем цвет рисования - белый для текста
                sdlgfx_string(60, 20, rgb_text);               // Выводим строку RGB на экран - функция sdlgfx_string из sdlgfx.h
                break;                                        // Выходим из цикла, так как цвет RGB уже отображен
            }
        }

        sdlgfx_color(255, 255, 255); // Устанавливаем цвет рисования - белый для текста
        char figure_info_text[200]; // Объявляем буфер для строки информации о фигуре
        sprintf(figure_info_text, "Figure: %s, Scale: %.2f, Offset: (%d, %d)", // Форматируем строку информации о фигуре, включая имя, масштаб и смещение
                figures[current_figure_index].name, scale, x_offset, y_offset);
        sdlgfx_string(50, 560, figure_info_text); /* Смещено вниз на 20 пикселей */ 
             // Выводим информацию о фигуре на экран - функция sdlgfx_string из sdlgfx.h, смещено вниз

        if (show_countdown) { // Если включен показ countdown (автоматическая смена фигур)
            unsigned int current_time = SDL_GetTicks(); // Получаем текущее время в миллисекундах
            if (current_time - last_figure_change_time >= figure_change_interval_ms) { // Если прошло время, заданное интервалом смены фигуры
                current_figure_index++; // Переключаемся к следующей фигуре
                if (current_figure_index >= num_figures) current_figure_index = 0; // Если индекс вышел за границы, возвращаемся к первой фигуре
                last_figure_change_time = current_time; // Запоминаем время последней смены фигуры
                show_countdown = 1;                     // Продолжаем показывать countdown после автосмены
            }
        }

        if (show_countdown) { // Если нужно показывать countdown
            countdown_seconds = (figure_change_interval_ms - (SDL_GetTicks() - last_figure_change_time)) / 1000; // Вычисляем оставшееся время в секундах
            if (countdown_seconds < 0) countdown_seconds = 0; // Чтобы не было отрицательных значений
            char countdown_text[70]; // Объявляем буфер для строки countdown
            sprintf(countdown_text, "Next figure in %d sec... (N/P or 1-9/0 to change now)", countdown_seconds); // Форматируем строку countdown
            sdlgfx_color(255, 255, 255); // Устанавливаем цвет рисования - белый для текста
            sdlgfx_string(50, 580, countdown_text); /* Смещено вниз на 20 пикселей */ 
            // Выводим строку countdown на экран - функция sdlgfx_string из sdlgfx.h, смещено вниз
        }

        sdlgfx_flush(); // Обновляем экран, отображая все нарисованное - функция из sdlgfx.h
        SDL_Delay(25);  // Пауза 25 миллисекунд (примерно 40 кадров в секунду) для управления частотой кадров
    }

    sdlgfx_close(); // Закрываем окно SDL и освобождаем ресурсы - функция из sdlgfx.h
    SDL_Quit();     // Завершаем работу подсистемы SDL
    return 0;       // Возвращаем 0, указывая на успешное завершение программы
}
