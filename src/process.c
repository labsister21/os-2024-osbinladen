#include "header/process/process.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"

ProcessManagerState process_manager_state = {
    .active_process_count = 0,
    .process_list = {{0}},
};
static int32_t next_pid = 1; 

int32_t process_generate_new_pid() {
    return next_pid++;
}

int32_t process_list_get_inactive_index() {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (process_manager_state.process_list[i].metadata.state == TERMINATED) {
            return i;
        }
    }
    return -1;  
}

uint32_t ceil_div(uint32_t numerator, uint32_t divisor) {
    if (divisor == 0) {
        return 0; 
    }
    return (numerator + divisor - 1) / divisor;
}

struct ProcessControlBlock* process_get_current_running_pcb_pointer() {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (process_manager_state.process_list[i].metadata.state == RUNNING) {
            return &process_manager_state.process_list[i];
        }
    }
    return NULL; 
}

bool process_destroy(uint32_t pid) {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (process_manager_state.process_list[i].metadata.pid == pid && process_manager_state.process_list[i].metadata.state != TERMINATED) {

            process_manager_state.process_list[i].metadata.state = TERMINATED;

            for (uint32_t j = 0; j < process_manager_state.process_list[i].memory.page_frame_used_count; j++){
                if (!paging_free_user_page_frame(
                        (struct PageDirectory*) process_manager_state.process_list[i].context.page_directory_virtual_addr, 
                        process_manager_state.process_list[i].memory.virtual_addr_used[j])
                    ){
                    return false;
                }
            }

            if (!paging_free_page_directory((struct PageDirectory*) process_manager_state.process_list[i].context.page_directory_virtual_addr)) {
                return false;
            }

            memset(&process_manager_state.process_list[i], 0, sizeof(ProcessControlBlock));
            process_manager_state.active_process_count--;
            return true;
        }
    }
    return false;
}
int32_t process_create_user_process(struct FAT32DriverRequest request) {
    int32_t retcode = PROCESS_CREATE_SUCCESS; 
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t) request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE) {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    struct PageDirectory* allocatedPage = paging_create_new_page_directory();
    paging_allocate_user_page_frame(allocatedPage, request.buf);

    // Process PCB 
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(process_manager_state.process_list[p_index]);

    new_pcb->metadata.pid = process_generate_new_pid();
    memcpy(new_pcb->metadata.process_name, request.name, 8);
    new_pcb->memory.page_frame_used_count = page_frame_count_needed;

    new_pcb->context.page_directory_virtual_addr = (uint32_t) allocatedPage;
    new_pcb->metadata.state = NEW;
    new_pcb->context.cpu.eflags = CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;

    struct PageDirectory* currentPageDirectory = paging_get_current_page_directory_addr();

    paging_use_page_directory(allocatedPage);
    int fsretcode = read(request);

    if (fsretcode != 0){
        process_destroy(new_pcb->metadata.pid);
        paging_use_page_directory(currentPageDirectory);
        retcode = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        goto exit_cleanup;
    }

    paging_use_page_directory(currentPageDirectory);

    new_pcb->context.cpu.esp = 0x400000 - 4;
    new_pcb->context.cpu.cs = 0x1b;
    new_pcb->context.cpu.ds = 0x23;
    new_pcb->context.cpu.es = 0x23;
    new_pcb->context.cpu.ss = 0x23;
    new_pcb->context.cpu.fs = 0x23;
    new_pcb->context.cpu.gs = 0x23;
    process_manager_state.active_process_count++;

exit_cleanup:
    return retcode;
}

void process_get_processes_info(ProcessMetadata* arr){
    uint32_t arr_index = 0;

    for(uint32_t i = 0; i < PROCESS_COUNT_MAX; i++){
        if (process_manager_state.process_list->metadata.state != TERMINATED){
            ProcessMetadata* currentMetadata = &process_manager_state.process_list[i].metadata;
            arr[arr_index].pid = currentMetadata->pid;
            memcpy(arr[arr_index].process_name, currentMetadata->process_name, 32);
            arr[arr_index].state = currentMetadata->state;
            arr[arr_index].priority = currentMetadata->priority;
            arr_index++;
        }
    }
}