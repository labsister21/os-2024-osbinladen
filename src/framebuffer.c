#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/driver/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

void framebuffer_set_cursor(uint16_t r, uint16_t c) {
    uint16_t pos = r * SCREEN_WIDTH + c;
 
	out(CURSOR_PORT_CMD, 0x0F);
	out(CURSOR_PORT_DATA, (uint16_t) (pos & 0xFF));
	out(CURSOR_PORT_CMD, 0x0E);
	out(CURSOR_PORT_DATA, (uint16_t) ((pos >> 8) & 0xFF));
}

void framebuffer_write(uint16_t row, uint16_t col, char c, uint8_t fg, uint8_t bg) {
    framebuffer_set_cursor(row, col);
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (volatile uint16_t *)0xB8000 + (row * SCREEN_WIDTH + col) ;
    *where = c | (attrib << 8);
}

void framebuffer_clear(void) {
    memset(FRAMEBUFFER_MEMORY_OFFSET,0x0700, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
}

uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    out(0x3D4, 0x0F);
    pos |= in(0x3D5);
    out(0x3D4, 0x0E);
    pos |= ((uint16_t)in(0x3D5)) << 8;
    return pos;
}