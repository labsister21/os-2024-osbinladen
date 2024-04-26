ENTRY(loader)                /* the name of the entry label */
/* Relocation address 0xC0010000, but load address (physical location) 0x100000 */

/* Real mode loads at 0x00001000 */
REAL_BASE = 0x00001000;
KERNEL_OFFSET = 0xC0000000;

SECTIONS {
    . = 0xC0100000;          /* use relocation address at 0xC0100000 */
    /* Linker variable that can be used in kernel */
    _linker_kernel_virtual_addr_start  = .; 
    _linker_kernel_physical_addr_start = . - 0xC0000000; 
    .multiboot ALIGN (0x1000) : AT (ADDR (.multiboot) - 0xC0000000) {
        *(.multiboot)        /* put GRUB multiboot header at front */
    }

    .setup.text ALIGN (0x1000) : AT (ADDR (.setup.text) - 0xC0000000) {
        *(.setup.text)       /* initial setup code */
    }

    /* This is the tricky part. The LMA (load memory address) is the
     * memory location the code/data is read into memory by the
     * multiboot loader. The LMA is after the colon. We want to tell
     * the linker that the code/data in this section was loaded into
     * RAM in the memory area above 0x100000. On the other hand the
     * VMA (virtual memory address) specified before the colon acts
     * like an ORG directive. The VMA tells the linker to resolve all
     * subsequent code starting relative to the specified VMA. The
     * VMA in this case is REAL_BASE which we defined as 0x1000.
     * 0x1000 is 4KB page aligned (useful if you ever use paging) and
     * resides above the end of the interrupt table and the
     * BIOS Data Area (BDA)
     */

    __physreal_diff = . - REAL_BASE;
    .realmode REAL_BASE : AT(ADDR(.realmode) + __physreal_diff - KERNEL_OFFSET) {
        /* The __realmode* values can be used by code to copy
         * the code/data from where it was placed in RAM by the
         * multiboot loader into lower memory at REAL_BASE
         *
         * . (period) is the current VMA */
        __realmode_vma_start = .;

        /* LOADADDR is the LMA of the specified section */
        __realmode_lma_start = LOADADDR(.realmode);
        *(.text.realmode);
        *(.data.realmode);
    }
    . = ALIGN(4);
    __realmode_vma_end = .;
    __realmode_secsize   = ((__realmode_vma_end)-(__realmode_vma_start));
    __realmode_secsize_l = __realmode_secsize>>2;
    __realmode_lma_end   = __realmode_vma_start + __physreal_diff + __realmode_secsize - KERNEL_OFFSET;

    /* . (period) is the current VMA. We set it to the value that would
     * have been generated had we not changed the VMA in the previous
     * section. The .text section also specified the LMA = VMA with
     * AT(ADDR(.text))
     */
    . += __physreal_diff;

    .text ALIGN (0x1000) : AT (ADDR (.text) - 0xC0000000) {
        *(.text)             /* all text sections from all files */
    }

    .rodata ALIGN (0x1000) : AT (ADDR (.rodata) - 0xC0000000) {
        *(.rodata*)          /* all read-only data sections from all files */
    }

    .data ALIGN (0x1000) : AT (ADDR (.data) - 0xC0000000) {
        *(.data)             /* all data sections from all files */
    }

    .bss ALIGN (0x1000) : AT (ADDR (.bss) - 0xC0000000) {
        *(COMMON)            /* all COMMON sections from all files */
        *(.bss)              /* all bss sections from all files */
        [a-z\/]*kernel-entrypoint.o(.bss)
        _linker_kernel_stack_top = .;
    }
    /* Linker variable that can be used in kernel */
    _linker_kernel_virtual_addr_end  = .;
    _linker_kernel_physical_addr_end = . - 0xC0000000;
}
