#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/cpu/portio.h"
#include "header/cpu/interrupt.h"
#include "header/interrupt/idt.h"
#include "header/driver/keyboard.h"
#include "header/driver/disk.h"
#include "header/filesystem/fat32.h"
#include "header/realModeGaming.h"
#include "header/driver/graphics.h"
#include "header/memory/paging.h"
#include "archive_src/header/framebuffer.h"
#include "imgdata/attack.h"

//void kernel_setup(void) {
//     uint32_t a;
//     uint32_t volatile b = 0x0000BABE;
//     __asm__("mov $0xCAFE0000, %0" : "=r"(a));
//     while (true) b += 1;
// }

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     while (true);
// }

// Treasure
// void kernel_setup(void) {
//     framebuffer_clear();
//     char art[] = "       _,,--,,_\n"
//                  "     /`       .`\\\n"
//                  "    /  '  _.-'   \\\n"
//                  "    |  `'_{}_    |    SORRY ABOUT THOSE 72 VIRGINS\n"
//                  "    |  /`    `\\  |\n"
//                  "     \\/ ==  == \\/     WE REPLACED THEM WITH 72 GAY BEARS\n"
//                  "     /| (.)(.) |\\     FROM SAN FRANCISCO\n"
//                  "     \\|  __)_  |/\n"
//                  "      |\\/____\\/|      YOU'LL GET ALONG FAMOUSLY\n"
//                  "      | ` ~~ ` |\n"
//                  "      \\        /              OS BIN LADEN\n"
//                  "       `.____.`";

//     // Start position
//     uint8_t row = 5; // Starting row
//     uint8_t col = 20; // Starting column

//     // Foreground and background colors
//     uint8_t fg = 15; // White
//     uint8_t bg = 0;  // Black

//     // Write each character of the art to the framebuffer
//     for (int i = 0; art[i] != '\0'; i++) {
//         if (art[i] == '\n') {
//             row++; // Move to the next row
//             col = 20; // Reset column position
//             continue;
//         }
//         framebuffer_write(row, col++, art[i], fg, bg);
//     }
//     while (true);
// }

// void kernel_setup(void) {
//     framebuffer_clear();
//     framebuffer_write(3, 8,  'H', 0, 0xF);
//     framebuffer_write(3, 9,  'a', 0, 0xF);
//     framebuffer_write(3, 10, 'i', 0, 0xF);
//     framebuffer_write(3, 11, '!', 0, 0xF);
//     framebuffer_set_cursor(3, 10);
//     while (true);
// }

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);
        
//     int col = 0;
//     keyboard_state_activate();
//     while (true) {
//          char c;
//          get_keyboard_buffer(&c);
//          if (c) framebuffer_write(0, col++, c, 0xF, 0);
//     }
// }


// void kernel_setup(void) {
//     realmode_setup();
//     load_gdt(&_gdt_gdtr);

//     push_state();
//     // 4F02h : Set video mode, Mode : 117h
//     bios_10h_interrupt(0x4F02, 0x117 | 0x4000, 0, 0, 0);
//     pop_state();

//     pic_remap();
//     activate_keyboard_interrupt();
//     initialize_idt();

//     initialize_filesystem_fat32();

//     char buf[1];

//     struct FAT32DriverRequest req = {
//         buf,
//         {'f', 'o', 'n', 'd', 0x0, 0x0, 0x0, 0x0},
//         {0x0, 0x0, 0x0},
//         2,
//         0
//     };

//     write(req);

//     while (true);
// }

        
//     int col = 0;
//     keyboard_state_activate();
//     while (true) {
//          char c;
//          get_keyboard_buffer(&c);
//          if (c) framebuffer_write(0, col++, c, 0xF, 0);
//     }
// }

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);

    pic_remap();
    initialize_idt();
    keyboard_state_activate();
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t*) 0);

    int row, col;
    int last_pos = 0;

    for(row = 0; row < GRAPHICS_HEIGHT; row++){
        for(col = 0; col < GRAPHICS_WIDTH; col++){
            draw_pixel_at_with_code(row, col, title[row*GRAPHICS_WIDTH + col]);
        }
    }

    while (true){
            while (keyboard_state.buffer_index != last_pos){
            if (keyboard_state.keyboard_buffer[last_pos] == ' '){
                goto escape;
            }
            last_pos += (keyboard_state.buffer_index > last_pos) ? 1 : -1;
        }
    }

    escape:
    reset_keyboard_buffer();
    last_pos = 0;

    set_screen_color(BLACK);

    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };
    read(request);

    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);

    while (true);
}

//     // int row, col;
//     // int last_pos = 0;

//     // for(row = 0; row < GRAPHICS_HEIGHT; row++){
//     //     for(col = 0; col < GRAPHICS_WIDTH; col++){
//     //         draw_pixel_at_with_code(row, col, title[row*GRAPHICS_WIDTH + col]);
//     //     }
//     // }

//     // while (true){
//     //         while (keyboard_state.buffer_index != last_pos){
//     //         if (keyboard_state.keyboard_buffer[last_pos] == ' '){
//     //             goto escape;
//     //         }
//     //         last_pos += (keyboard_state.buffer_index > last_pos) ? 1 : -1;
//     //     }
//     // }

//     // escape:
//     // reset_keyboard_buffer();
//     // last_pos = 0;

//     set_screen_color(BLACK);

//     // while(true){
//     //     draw_char_at('_', keyboard_state.buffer_index / TEXT_WIDTH, keyboard_state.buffer_index % TEXT_WIDTH, WHITE, BLACK);
//     //     while (keyboard_state.buffer_index != last_pos){
//     //         draw_char_at(keyboard_state.keyboard_buffer[last_pos], last_pos/TEXT_WIDTH, last_pos % TEXT_WIDTH, WHITE, BLACK);
//     //         last_pos += (keyboard_state.buffer_index > last_pos) ? 1 : -1;
//     //     }
//     // }

//     initialize_filesystem_fat32();

//     char buf[1];

//     struct FAT32DriverRequest req = {
//         buf,
//         {'f', 'o', 'l', 'd', 'e', 'r', '2', 0x0},
//         {0x0, 0x0, 0x0},
//         0x02,
//         0
//     };

//     char a = delete(req);
    
//     int y = 1 + 1;
//     (void)y;
//     (void)a;
// }





// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     // framebuffer_clear();
//     // framebuffer_set_cursor(0, 0);
//     initialize_filesystem_fat32();
//     gdt_install_tss();
//     set_tss_register();

//     // Allocate first 4 MiB virtual memory
//     paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t*) 0);

//     // Write shell into memory
//     struct FAT32DriverRequest request = {
//         .buf                   = (uint8_t*) 0,
//         .name                  = "shell",
//         .ext                   = "\0\0\0",
//         .parent_cluster_number = ROOT_CLUSTER_NUMBER,
//         .buffer_size           = 0x100000,
//     };
//     read(request);

//     // Set TSS $esp pointer and jump into shell 
//     set_tss_kernel_current_stack();
//     kernel_execute_user_program((uint8_t*) 0);

//     while (true);
// }
