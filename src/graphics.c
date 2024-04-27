#include "header/driver/graphics.h"
#include "header/stdlib/string.h"
#include "header/realModeGaming.h"
#include "font/basicFont.h"
#include "header/driver/keyboard.h"

int last_pos = 0;

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

Color16 int_to_color(uint16_t color16){
    return (Color16){color16 & 0b11111, (color16 >> 5) & 0b111111, (color16 >> 11) & 0b11111};
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

void buffered_draw(char c, Color16 fg, Color16 bg){
    int last_pos = keyboard_state.buffer_index;
    key_handler(c);
    while (keyboard_state.buffer_index != last_pos){
        draw_char_at(keyboard_state.keyboard_buffer[last_pos], last_pos/TEXT_WIDTH, last_pos % TEXT_WIDTH, fg, bg);
        last_pos += (keyboard_state.buffer_index > last_pos) ? 1 : -1;
    }
}

void putchar(char c, uint16_t textColor){
    buffered_draw(c, int_to_color(textColor), BLACK);
}

void puts(char* buffer, int charCount, uint16_t textColor){
    for(int i = 0; i < charCount; i++){
        putchar(buffer[i], textColor);
    }
}

void draw_cursor(){
    draw_char_at('_', keyboard_state.buffer_index/TEXT_WIDTH, keyboard_state.buffer_index % TEXT_WIDTH, WHITE, BLACK);
}