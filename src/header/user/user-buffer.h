#ifndef USER_BUFFER_H
#define USER_BUFFER_H

#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"
#include "header/driver/graphics.h"
#include "header/user/user-shell.h"

extern struct ShellState main_state;

uint16_t color_to_int(Color16 color);
Color16 int_to_color(uint16_t color16);
void printToScreen(char* msg, uint16_t color);
void reset_user_buffer();
bool isBlank(char c);
void get_next_word();
void cmdHandler();
void handle_newline();
void handle_tab();
void handle_backspace();
void handle_others(char key);
void inputChar(char c);
void deleteLine();
void appendChar();
void handle_down_arrow();
int getArrayLength(char *arr[], int maxSize);
void handle_up_arrow();

#endif