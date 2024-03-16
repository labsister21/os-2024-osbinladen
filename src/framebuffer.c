#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/driver/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    uint16_t pos = r * SCREEN_WIDTH + c;
    // framebuffer_write(r, c, ' ', 0xF, 0);
	out(CURSOR_PORT_CMD, 0x0F);
	out(CURSOR_PORT_DATA, (uint8_t) (pos & 0xFF));
	out(CURSOR_PORT_CMD, 0x0E);
	out(CURSOR_PORT_DATA, (uint8_t) ((pos >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    framebuffer_set_cursor(row, col + 1);
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (volatile uint16_t *)0xB8000 + (row * SCREEN_WIDTH + col) ;
    *where = c | (attrib << 8);
}

void* better_memset(void *s, int c, size_t n) {
    uint16_t *buf = (uint16_t*) s;
    for (size_t i = 0; i < n; i++)
        buf[i] = (uint16_t) c;
    return s;
}

void framebuffer_clear(void) {
    better_memset(FRAMEBUFFER_MEMORY_OFFSET, 0x0F20, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
}

uint16_t framebuffer_get_cursor(void)
{
    uint16_t pos = 0;
    out(CURSOR_PORT_CMD, 0x0F);
    pos |= in(0x3D5);
    out(CURSOR_PORT_CMD, 0x0E);
    pos |= ((uint16_t)in(0x3D5)) << 8;
    return pos;
}