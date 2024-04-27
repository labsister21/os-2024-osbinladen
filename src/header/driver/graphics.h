#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define VIDMEM ((uint16_t*) 0xfd000000)
#define GRAPHICS_WIDTH 1024
#define GRAPHICS_HEIGHT 768
#define GRAPHICS_SIZE (GRAPHICS_WIDTH*GRAPHICS_HEIGHT)
#define PIXEL_SIZE 2
#define TEXT_WIDTH (GRAPHICS_WIDTH/8)
#define TEXT_HEIGHT (GRAPHICS_HEIGHT/16)
#define TEXT_SIZE (TEXT_WIDTH * TEXT_HEIGHT)

#define BLACK (Color16){0, 0, 0}
#define BLUE (Color16){0b11111, 0, 0}
#define GREEN (Color16){0, 0b111111, 0}
#define RED (Color16){0, 0, 0b11111}
#define WHITE (Color16){0b11111, 0b111111, 0b11111}

typedef struct color16 {
    // 5 bit red, 6 bit green, 5 bit blue
    uint8_t red: 5;
    uint8_t green: 6;
    uint8_t blue: 5;
} Color16;

typedef struct color24 {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGBColor;

uint16_t color_to_int(Color16 color);
Color16 int_to_color(uint16_t Color16);
void set_screen_color(Color16 color);
void* word_memset(void *s, int c, size_t n);

void draw_pixel_at(int row, int column, Color16 color);
void draw_pixel_at_with_code(int row, int column, uint16_t color);
void draw_char_at(char c, int row, int column, Color16 fg, Color16 bg);
int row_pos_to_pixel_row(int row);
int col_pos_to_pixel_col(int col);
void draw_null_char(int row, int column, Color16 bg);
void buffered_draw(char c, Color16 fg, Color16 bg);
void putchar(char c, uint16_t textColor);
void puts(char* buffer, int charCount, uint16_t textColor);
void draw_cursor();

#endif
