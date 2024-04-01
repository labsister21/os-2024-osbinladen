; This piece of assembly code implement the bios_10h_interupt services which
; switch the cpu into real mode and then call the bios_10h_interrupt
; this code is adapted from the following sources
; 1. Intel® 64 and IA-32 Architectures Software Developer’s Manual Volume 3A:
; Chapter 10.9
; 2. Nanobyte's video on protected mode : https://www.youtube.com/watch?v=EBdzWFyKZ0U
; 3. This stackoverflow thread https://stackoverflow.com/questions/41563879/ld-errors-while-linking-16-bit-real-mode-code-into-a-multiboot-compliant-elf-exe
; 4. Grub 0.95 legacy file grub-0.95/stage2/asm.S

CODE32SEL equ 0x08
DATA32SEL equ 0x10
CODE16SEL equ 0x18
DATA16SEL equ 0x20
REALSTACK equ 0x2000 - 0x10

global bios_10h_interrupt
global push_state
global pop_state

bits 16
section .data.realmode

save_idt: dw 0                  ; 
          dd 0

save_esp: dd 0                  ; Save original stack pointer

real_ivt: dw (256 * 4) - 1      ; Realmode IVT has 256 CS:IP pairs
          dd 0                  ; Realmode IVT physical address at address 0x00000

save_origin_address: dd 0       ; Save Origin Address (currently not used)

interrupt_args: dw 0                     ; ax
                dw 0                     ; bx
                dw 0                     ; cx
                dw 0                     ; es
                dw 0                     ; di

save_esp_address: dd 0

save_state:
    dd 0                ; eax
    dd 0                ; ecx
    dd 0                ; edx
    dd 0                ; ebx
    dd 0                ; esi
    dd 0                ; edi
    dd 0                ; flags

save_status:
    dw 0

align 4

bits 32
section .text

push_state:

    ; Save registers
    mov [save_state], eax
    mov [save_state + 4], ecx
    mov [save_state + 8], edx
    mov [save_state + 12], ebx
    mov [save_state + 16], esi
    mov [save_state + 20], edi

    ; Save flags
    pushf
    mov eax, [esp]
    mov [save_state + 24], eax
    popf

    ; return eax value
    mov eax, [save_state]

    ret

bios_10h_interrupt:

    ; disable interrupt (step 1)
    cli

    ; Paging is disabled (step 2 skipped)

    ; Save call args
    mov ax, [esp + 4]
    mov [interrupt_args], ax
    mov ax, [esp + 8]
    mov [interrupt_args + 2], ax
    mov ax, [esp + 12]
    mov [interrupt_args + 4], ax
    mov ax, [esp + 16]
    mov [interrupt_args + 6], ax
    mov ax, [esp + 20]
    mov [interrupt_args + 8], ax

    ; Save return address
    mov eax, [esp]
    mov [save_origin_address], eax

    ; Save stack
    mov [save_esp], esp

    ; Save IDT
    sidt [save_idt]

    ; Long jump to 16 bit protected mode (step 4)
    jmp CODE16SEL:prot16

bits 16
; 16 bits code
section .text.realmode
prot16:
    ; 16 bit protected mode loads the cs register with real mode segment limit

    ; Load segment register (step 4)
    mov ax, DATA16SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Load real mode IVT (Step 5)
    lidt [real_ivt]

    ; Clear PE Flag (Step 6)
    mov eax, cr0
    and eax, ~1
    mov cr0, eax

    ; Far jump to real mode (Step 7)
    jmp 0:real_mode

real_mode:

    ; Now we are in real mode

    ; Sets real mode stack to grow down from 0x1FF0:0xFFFF
    mov ax, REALSTACK
    mov sp, ax
    mov bp, ax

    mov ss, ax

    ; Clear segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Call the interrupt
    sti

    mov ax, [interrupt_args]
    mov bx, [interrupt_args + 2]
    mov cx, [interrupt_args + 4]
    mov es, [interrupt_args + 6]
    mov di, [interrupt_args + 8]

    int 0x10

    xor bx, bx
    mov es, bx
    mov [save_status], ax

    jmp done

done:
    cli
    mov eax,cr0
    inc eax
    mov cr0,eax
    jmp dword CODE32SEL:return_to_prot

bits 32
section .text

return_to_prot:

    mov eax, DATA32SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov dword esp, [save_esp]
    lidt [save_idt]

    mov eax, 0xCAFEBABE         ; Return pointer to mode_info structure
    ret

pop_state:
    ; go to past data
    mov eax, [save_state]
    mov ecx, [save_state + 4]
    mov edx, [save_state + 8]
    mov ebx, [save_state + 12]
    mov esi, [save_state + 16]
    mov edi, [save_state + 20]

    push dword [save_state + 24]
    popf

    ret