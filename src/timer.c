#include "header/driver/timer.h"
#include "header/scheduler/scheduler.h"
#include "header/process/process.h"
#include "header/cpu/interrupt.h"

uint32_t tick_counter = 0;
uint32_t second_counter = 0;

void timer_isr(struct InterruptFrame frame){

    tick_counter++;
    if (tick_counter >= 1000){
        second_counter++;
        tick_counter -= 1000;
    }
    pic_ack(PIC1_OFFSET + IRQ_TIMER);

    if (process_manager_state.active_process_count > 0){
        ProcessControlBlock* currPcb = process_get_current_running_pcb_pointer();
        Context currContext = currPcb->context;
        currContext.cpu.eax = frame.cpu.general.eax;
        currContext.cpu.ebx = frame.cpu.general.ebx;
        currContext.cpu.ecx = frame.cpu.general.ecx;
        currContext.cpu.edx = frame.cpu.general.edx;
        currContext.cpu.esp = frame.cpu.stack.esp;
        currContext.cpu.ebp = frame.cpu.stack.ebp;
        currContext.cpu.edi = frame.cpu.index.edi;
        currContext.cpu.esi = frame.cpu.index.esi;
        currContext.cpu.gs = frame.cpu.segment.gs;
        currContext.cpu.fs = frame.cpu.segment.fs;
        currContext.cpu.es = frame.cpu.segment.es;
        currContext.cpu.ds = frame.cpu.segment.ds;
        currContext.cpu.eip = frame.int_stack.eip;
        scheduler_save_context_to_current_running_pcb(currContext);
        scheduler_switch_to_next_process();
    }

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