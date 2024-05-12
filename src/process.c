#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"

ProcessManagerState process_manager_state;

static int32_t next_pid = 1; 

int32_t process_generate_new_pid() {
    return next_pid++;
}

int32_t process_list_get_inactive_index() {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (process_manager_state.process_list[i].metadata.state == TERMINATED || process_manager_state.process_list[i].metadata.state == NEW) {
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

            if (!paging_free_page_directory(process_manager_state.process_list[i].context.page_directory_virtual_addr)) {
                return false;
            }

            memset(&process_manager_state.process_list[i], 0, sizeof(ProcessControlBlock));  // Use memset to zero out the PCB
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

    // Process PCB 
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(process_manager_state.process_list[p_index]);

    new_pcb->metadata.pid = process_generate_new_pid();

exit_cleanup:
    return retcode;
}