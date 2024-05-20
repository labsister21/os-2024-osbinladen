#include "header/scheduler/scheduler.h"
#include "header/cpu/interrupt.h"
#include "header/memory/paging.h"

int current_process_list_id = -1;

/**
 * Read all general purpose register values and set control register.
 * Resume the execution flow back to ctx.eip and ctx.eflags
 * 
 * @note          Implemented in assembly
 * @param context Target context to switch into
 */
__attribute__((noreturn)) extern void process_context_switch(struct Context ctx);


/* --- Scheduler --- */
/**
 * Initialize scheduler before executing init process 
 */
void scheduler_init(void){
    current_process_list_id = -1;
}

/**
 * Save context to current running process
 * 
 * @param ctx Context to save to current running process control block
 */
void scheduler_save_context_to_current_running_pcb(struct Context ctx){
    process_manager_state.process_list[current_process_list_id].context = ctx;
}

/**
 * Trigger the scheduler algorithm and context switch to new process
 */
__attribute__((noreturn)) void scheduler_switch_to_next_process(void){
    if (current_process_list_id != -1){
        process_manager_state.process_list[current_process_list_id].metadata.state = READY;
    }
    current_process_list_id++;

    while(process_manager_state.process_list[current_process_list_id].metadata.state != READY){
        if(process_manager_state.process_list[current_process_list_id].metadata.state == NEW){
            process_manager_state.process_list[current_process_list_id].metadata.state = READY;
        }
        current_process_list_id++;
        if (current_process_list_id >= PROCESS_COUNT_MAX){
            current_process_list_id -= PROCESS_COUNT_MAX;
        }
    }

    process_manager_state.process_list[current_process_list_id].metadata.state = RUNNING;
    paging_use_page_directory((struct PageDirectory*) process_manager_state.process_list[current_process_list_id].context.page_directory_virtual_addr);
    process_context_switch(process_manager_state.process_list[current_process_list_id].context);
}