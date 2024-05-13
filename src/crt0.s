extern main

_start:
	call main
    mov ebx, eax
	mov eax, 13   ; Assuming syscall exit is 10
	int 0x30