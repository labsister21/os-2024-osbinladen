ENTRY(loader);

/* Multiboot spec uses 0x00100000 as a base */
PHYS_BASE = 0x00100000;
REAL_BASE = 0x00001000;

SECTIONS
{
    . = PHYS_BASE;

    /* Place the multiboot record first */
    .multiboot : {
        *(.multiboot);
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
    .realmode REAL_BASE : AT(ADDR(.realmode) + __physreal_diff) {
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
    __realmode_lma_end   = __realmode_vma_start + __physreal_diff + __realmode_secsize;

    /* . (period) is the current VMA. We set it to the value that would
     * have been generated had we not changed the VMA in the previous
     * section. The .text section also specified the LMA = VMA with
     * AT(ADDR(.text))
     */
    . += __physreal_diff;
    .text ALIGN(4K): AT(ADDR(.text)) {
        *(.text);
    }

    /* From this point the linker script is typical */
    .data ALIGN(4K) : {
        *(.data);
    }

    .data ALIGN(4K) : {
        *(.rodata);
    }

    /* We want to avoid this section being placed in low memory */
    .eh_frame : {
        *(.eh_frame*);
    }

    .bss ALIGN(4K): {
        *(COMMON);
        *(.bss)
    }

    /* The .note.gnu.build-id section will usually be placed at the beginning
     * of the ELF object. We discard it (if it is present) so that the
     * multiboot header is placed as early as possible in the file. The
     * multiboot header must appear in the first 8K and be on a 4 byte
     * aligned offset per the multiboot spec.
     */
    /DISCARD/ : {
        *(.note.gnu.build-id);
        *(.comment);
    }
}

/*
* Linker script taken from this stackoverflow thread:
* https://stackoverflow.com/questions/41563879/ld-errors-while-linking-16-bit-real-mode-code-into-a-multiboot-compliant-elf-exe
*/