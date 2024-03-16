#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/driver/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/cpu/interrupt.h"
#include "header/interrupt/idt.h"
// void kernel_setup(void) {
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

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    // framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    __asm__("int $0x4");
    while (true);
}



