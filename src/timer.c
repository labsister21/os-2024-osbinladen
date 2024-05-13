#include "header/driver/timer.h"

uint32_t tick_counter = 0;
uint32_t second_counter = 0;

void timer_isr(){
    tick_counter++;
    if (tick_counter >= 1000){
        second_counter++;
        tick_counter -= 1000;
    }
    pic_ack(PIC1_OFFSET + IRQ_TIMER);
}
void init_counter(){
    tick_counter = 0;
    second_counter = 0;
}

uint32_t timer_get_second(){
    return second_counter;
}

uint32_t timer_get_tick(){
    return tick_counter;
}