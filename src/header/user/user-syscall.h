#ifndef _USER_SYSCALL
#define _USER_SYSCALL

#include <stdint.h>

// ============================================ BATAS SUCI ===========================================
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

#endif