#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../cpu/interrupt.h"

extern uint32_t tick_counter;
extern uint32_t second_counter;

void timer_isr(struct InterruptFrame frame);
void init_counter();
uint32_t timer_get_second();
uint32_t timer_get_tick();

#endif
