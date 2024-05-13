#ifndef USER_SHELL_H
#define USER_SHELL_H

#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"
#include "header/driver/graphics.h"

struct ShellState {
    char    userBuffer[TEXT_HEIGHT*TEXT_WIDTH];
    char    stringBuffer[256][256];
    int     userBufferPos;
    int     readPointer;
    char    currentWord[64];
    int     cwd_cluster_number;
    int     last_index_arrow;
};

extern struct ShellState main_state;

int main(void);

#endif