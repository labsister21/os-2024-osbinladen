#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"
#include "header/driver/graphics.h"


void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

uint16_t color_to_int(Color16 color);

Color16 int_to_color(uint16_t color16);

struct ShellState {
    char    userBuffer[TEXT_HEIGHT*TEXT_WIDTH];
    int     userBufferPos;
    int     readPointer;
    char    currentWord[64];
    int     cwd_cluster_number;
};

void reset_user_buffer();

bool isBlank(char c);

void get_next_word();

void cmdHandler();

void handle_newline();

void handle_tab();

void handle_backspace();

void handle_others(char key);

void inputChar(char c);

void printToScreen(char* msg, uint16_t color);

void print_cur_dir(struct FAT32DirectoryTable dir_table);

int ls();

int mkdir(char *goal, int goalLength);

void printNewLine();

