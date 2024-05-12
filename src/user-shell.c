#include "header/user/user-shell.h"
#include "header/user/shell_cmd.h"
#include "header/user/user-buffer.h"

struct ShellState main_state = {
    .userBuffer = {0},
    .stringBuffer = {0, 0},
    .currentWord = {0},
    .readPointer = 0,
    .userBufferPos = 0,
    .cwd_cluster_number = 0x2,
    .last_index_arrow = 0,
};

int main(void) {
    struct ClusterBuffer      cl[16]   = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };

    


    int32_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    if (retcode == 0)
        syscall(6, (uint32_t) "Let's blow up the world\n\n", 26, color_to_int(GREEN));

    char buffen;
    reset_user_buffer();
    syscall(7, 0, 0, 0);
    printToScreen("user@OSBINLADEN: ", color_to_int(GREEN));
    print_path_from_root(main_state.cwd_cluster_number, color_to_int(LIGHT_BLUE));
    printToScreen(" ", color_to_int(BLACK));
    while (true) {
        syscall(8, 0, 0, 0);
        syscall(4, (uint32_t) &buffen, 0, 0);
        inputChar(buffen);
    }

    return 0;
}