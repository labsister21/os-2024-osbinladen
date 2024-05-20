#include "header/user/clock/user-clock.h"
#include "header/user/user-syscall.h"

int main(){
    while(1){
        syscall(16, 0, 0, 0);
    }
}