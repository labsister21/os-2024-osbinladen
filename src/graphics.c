#include "header/driver/graphics.h"
#include "header/stdlib/string.h"
#include "header/realModeGaming.h"
#include "font/basicFont.h"

void* word_memset(void *s, int c, size_t n) {
    uint16_t *buf = (uint16_t*) s;
    for (size_t i = 0; i < n; i++)
        buf[i] = (uint16_t) c;
    return s;
}

void set_screen_color(Color16 color){
    word_memset(VIDMEM, color_to_int(color), GRAPHICS_SIZE);
}

uint16_t color_to_int(Color16 color){
    return color.red + (color.green << 5) + (color.blue << 11);
}

void draw_pixel_at(int row, int column, Color16 color){
    *(VIDMEM + row*GRAPHICS_WIDTH + column) = color_to_int(color);
}

void draw_pixel_at_with_code(int row, int column, uint16_t color){
    *(VIDMEM + row*GRAPHICS_WIDTH + column) = color;
}

int text_row_to_pixel_row(int row){
    return row*16;
}

int text_col_to_pixel_col(int col){
    return col*8;
}

void draw_char_at(char c, int row, int column, Color16 fg, Color16 bg){
    int x, y;

    if (c < 32 || c > 126){
        draw_null_char(row, column, bg);
        return;
    }
    
    for(y = 0; y < 16; y++){
        for(x = 0; x < 8; x++){
            if (((font8x16[(int)c * 16 + y] >> x) & 1) == 1){
                draw_pixel_at(
                    text_row_to_pixel_row(row) + y,
                    text_col_to_pixel_col(column) + 7 - x,
                    fg
                );
            }
            else{
                draw_pixel_at(
                    text_row_to_pixel_row(row) + y,
                    text_col_to_pixel_col(column) + 7 - x,
                    bg
                );
            }
        }
    }
}

void draw_null_char(int row, int column, Color16 bg){
    int x, y;
    for(y = 0; y < 16; y++){
        for(x = 0; x < 8; x++){
            draw_pixel_at(
                text_row_to_pixel_row(row) + y,
                text_col_to_pixel_col(column) + 7 - x,
                bg
            );
        }
    }
}