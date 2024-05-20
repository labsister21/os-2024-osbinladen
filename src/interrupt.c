#include "header/cpu/interrupt.h"
#include "header/cpu/portio.h"
#include "header/driver/keyboard.h"
#include "header/driver/timer.h"
#include "header/driver/graphics.h"
#include "header/filesystem/fat32.h"
#include "header/driver/charframe.h"
#include "header/process/process.h"
#include "header/driver/cmos.h"
#include "header/scheduler/scheduler.h"

void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        case PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            break;
        case PIC1_OFFSET + IRQ_TIMER:
            timer_isr(frame);
            break;
        case 0x30:
            syscall(frame);
            break;
    }
}

void activate_keyboard_interrupt(void) {
    int thing = in(PIC1_DATA);
    (void)thing;
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

void activate_timer_interrupt(void) {
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) (pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) ((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));

    __asm__ volatile("sti");
}


// Task State Segment

struct TSSEntry _interrupt_tss_entry = {
    .ss0  = 0x10,
};

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}

void syscall(struct InterruptFrame frame) {
    switch (frame.cpu.general.eax) {
        case 0:
            *((int8_t*) frame.cpu.general.ecx) = read(
                *((struct FAT32DriverRequest*) frame.cpu.general.ebx)
            );
            break;
        case 1:
            *((int8_t*) frame.cpu.general.ecx) = read_directory(
                *((struct FAT32DriverRequest*) frame.cpu.general.ebx)
            );
            break;
        case 2:
            *((int8_t*) frame.cpu.general.ecx) = write(
                *((struct FAT32DriverRequest*) frame.cpu.general.ebx)
            );
            break;
        case 3:
            *((int8_t*) frame.cpu.general.ecx) = delete(
                *((struct FAT32DriverRequest*) frame.cpu.general.ebx)
            );
            break;
        case 4:
            get_keyboard_buffer((char*) frame.cpu.general.ebx);
            break;
        case 5:
            putchar(frame.cpu.general.ebx, frame.cpu.general.ecx);
            break;
        case 6:
            puts(
                (char*) frame.cpu.general.ebx, 
                frame.cpu.general.ecx, 
                frame.cpu.general.edx
            ); // Assuming puts() exist in kernel
            break;
        case 7: 
            keyboard_state_activate();
            break;
        case 8:
            draw_cursor();
            break;
        case 9:
            *((int8_t*) frame.cpu.general.ecx) = delete_all(
                *((struct FAT32DriverRequest*) frame.cpu.general.ebx)
            );
            break;
        case 10:
            struct FAT32DirectoryTable *dir_table = (struct FAT32DirectoryTable *)frame.cpu.general.ecx;
            get_curr_working_dir(frame.cpu.general.ebx, dir_table);
            break;
        case 11:
            set_screen_color(BLACK);
            reset_buffer();
            break;
        case 12:
            *((int8_t*) frame.cpu.general.edx) = move(
                *((struct FAT32DriverRequest*) frame.cpu.general.ebx), *((struct FAT32DriverRequest*) frame.cpu.general.ecx)
            );
            break;

        /*
         * Syscall 13  : Destroy process
         * $ebx        : pid to be destroyed
         * $ecx        : unused
         * $edx        : retcode address
         */
        case 13:
            *((int8_t*) frame.cpu.general.edx) = process_destroy(frame.cpu.general.ebx);
            break;
        /*
         * Syscall 14  : Create Process
         * $ebx        : request address
         * $ecx        : unused
         * $edx        : retcode address
         */
        case 14:
            process_create_user_process(*((struct FAT32DriverRequest*) frame.cpu.general.ebx));
            break;
        /*
         * Syscall 15  : get_all_processes
         * $ebx        : address to array of process metadata
         * $ecx        : unused
         * $edx        : unused
         */
       case 15:
            process_get_processes_info((ProcessMetadata*) frame.cpu.general.ebx);
            break;
        /*
         * Syscall 16  : show_time at bottom right
         * $ebx        : unused
         * $ecx        : unused
         * $edx        : unused
         */
        case 16:
            show_HMS();
            break;
        /*
         * Syscall 17  : draw title screen
         * $ebx        : unused
         * $ecx        : unused
         * $edx        : unused
         */
        case 17:
            draw_title_screen();
            break;
        /*
         * Syscall 18  : get time since program start
         * $ebx        : second pointer
         * $ecx        : milisecond pointer
         * $edx        : unused
         */
        case 18:
            *((uint32_t*)frame.cpu.general.ebx) = timer_get_second();
            *((uint32_t*)frame.cpu.general.ecx) = timer_get_tick();
            break;
        /*
         * Syscall 19  : clear screen
         * $ebx        : unused
         * $ecx        : unused
         * $edx        : unused
         */
        case 19:
            set_screen_color(BLACK);
            break;
        case 20:
            process_destroy(current_process_list_id);
            break;
    }
}
