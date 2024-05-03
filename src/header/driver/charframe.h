#ifndef CHARFRAME_H
#define CHARFRAME_H

#include "graphics.h"

extern char charBuffer[TEXT_HEIGHT*TEXT_WIDTH];
extern int pos;

int get_cursor_pos();
char get_char_buffer_at(int idx);
void reset_buffer();
void buffered_draw(char c, Color16 fg, Color16 bg);
void putchar(char c, uint16_t textColor);
void puts(char* buffer, int charCount, uint16_t textColor);
void draw_cursor();

#endif