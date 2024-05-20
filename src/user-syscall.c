#include "header/user/user-syscall.h"

void sleep(uint32_t secondDuration, uint32_t milisecondDuration){
    uint32_t startSecond, startMilisecond;
    uint32_t second, milisecond;

    syscall(18, &startSecond, &startMilisecond, 0);
    do {
        syscall(18, &second, &milisecond, 0);
    } while (1000*startSecond + 1000*secondDuration + startMilisecond + milisecondDuration - 1000*second - milisecond > 0);
}

// ============================================ BATAS SUCI ===========================================
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}