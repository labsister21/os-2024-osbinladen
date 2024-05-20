global process_context_switch

; typedef struct AllCPURegister {
;   unsigned int eax, ebx, ecx, edx;   // General purpose registers
;   unsigned int esi, edi;           // Index and pointer registers
;   unsigned int esp, ebp;           // Stack pointer and base pointer
;   unsigned int eip;                // Instruction pointer
;   unsigned int eflags;            // Flags register
;   unsigned int cs, ds, es, fs, gs, ss; // Segment registers
; } __attribute__((packed)) AllCPURegister;

; Load struct Context (CPU GP-register) then jump
; Function Signature: void process_context_switch(struct Context ctx);
process_context_switch:

    ; ini hasil gpt karena saya males nguli, di asmGaming udh pernah bikin yg mirip kok -dito

    ; Using iret (return instruction for interrupt) technique for privilege change -kit
    ; Save the base address of struct Context ctx on the stack -kit
    lea     ecx, [esp+0x04]   ; ecx points to the address of ctx (4 bytes due to pushed arguments) -gpt

    ; Move all general-purpose registers from the struct to their corresponding registers -gpt

    ; mov     eax, [ecx + 0x00]  ; Move eax value from ctx -gpt
    ; eax belakangan ya gpt, saya perlu buat nyimpen flags -dito

    mov     ebx, [ecx + 0x04]  ; Move ebx value from ctx -gpt

    ; mov     ecx, [ecx + 0x08]  ; Move ecx value from ctx -gpt
    ; actually gpt don't do that because it will overwrite the ctx address -dito

    mov     edx, [ecx + 0x0C]  ; Move edx value from ctx -gpt
    mov     esi, [ecx + 0x10]  ; Move esi value from ctx -gpt
    mov     edi, [ecx + 0x14]  ; Move edi value from ctx -gpt

    ; Move stack pointer (esp) and base pointer (ebp) from the struct (assuming stacked) -gpt
    ; mov     esp, [ecx + 0x18]  ; Move esp value from ctx -
    ; actually gpt, biar iret yang menangani -dito
    mov     ebp, [ecx + 0x1C]  ; Move ebp value from ctx (might need adjustment if ebp is pushed later) -gpt

    ; (Optional) Restore segment registers if used in the context switch (not common in modern systems) -gpt
    ; mov     cs, [ecx + 0x20]   ; Move cs value from ctx (if used) -gpt
    ; ... (similar for ds, es, fs, gs, ss) -gpt
    ; KENAPA OFFSETNYA 0x20 GPT ITUMA EIP -dito

    ; oke gpt, optional tapi saya bikin aja la -dito
    mov     ds, [ecx + 0x2c]
    mov     es, [ecx + 0x30]
    mov     fs, [ecx + 0x34]
    mov     gs, [ecx + 0x38]

    ; ; Restore the instruction pointer (eip) from the struct -gpt
    ; mov     eip, [ecx + 0x20]  ; Move eip value from ctx (assuming 32-bit architecture) -gpt
    ; actually gpt, kita push ke stack aja, nanti biar iret yang menangani -dito

    ; THE STACK -dito
    mov     eax, [ecx + 0x3C] ; SS
    push    eax
    mov     eax, [ecx + 0x18] ; ESP
    push    eax
    mov     eax, [ecx + 0x24] ; EFLAGS
    push    eax
    mov     eax, [ecx + 0x28] ; CS
    push    eax
    mov     eax, [ecx + 0x20] ; EIP
    push    eax

    ; eax dan ecx sudah selesai digunakan -dito
    mov     eax, [ecx + 0x00]  ; Move eax value from ctx -gpt, ditaruh di sini oleh dito
    mov     ecx, [ecx + 0x08]  ; Move ecx value from ctx -gpt, ditaruh di sini oleh dito

    ; Perform the context switch (instruction pointer and privilege change) -gpt
    iret                    ; Return from interrupt routine, restoring state and transferring control -gpt

; End of function -gpt