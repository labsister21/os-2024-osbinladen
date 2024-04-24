#include "header/cpu/gdt.h"
#include "header/cpu/interrupt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            0, //segment low
            0, //base low
            0, //base mid
            0, //type bit
            0, //non system
            0, //dpl
            0, //p
            0, //limit high
            0, //avl
            0, //l
            0, //db
            0, //g
            0 //base high
        },
        {
            /* Kernel Code Descriptor */
            0xFFFF, //segment low
            0, //base low
            0, //base mid
            0xA, //type bit
            1, //non system
            0, //dpl
            1, //p
            0xF, //limit high
            0, //avl
            0, //l
            1, //db
            1, //g
            0 //base high
        },
        {
            /*Kernel Data Descriptor*/
            0xFFFF, //segment low
            0, //base low
            0, //base mid
            0x2, //type bit
            1, //non system
            0, //dpl
            1, //p
            0xF, //limit high
            0, //avl
            0, //l
            1, //db
            1, //g
            0 //base high
        },
        {
            /* User Code Descriptor */
            0xFFFF, //segment low
            0, //base low
            0, //base mid
            0xA, //type bit
            1, //non system
            0x3, //dpl
            1, //p
            0xF, //limit high
            0, //avl
            0, //l
            1, //db
            1, //g
            0 //base high
        },
        {
            /* User Data Descriptor*/
            0xFFFF, //segment low
            0, //base low
            0, //base mid
            0x2, //type bit
            1, //non system
            0x3, //dpl
            1, //p
            0xF, //limit high
            0, //avl
            0, //l
            1, //db
            1, //g
            0 //base high
        },
        {
            .limit_high        = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .segment_low       = sizeof(struct TSSEntry),
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .non_system        = 0,    // S bit
            .type_bit          = 0x9,
            .dpl               = 0,    // DPL
            .p                 = 1,    // P bit
            .db                = 1,    // D/B bit
            .l                 = 0,    // L bit
            .g                 = 0,    // G bit
        },


        // for 16 bit protected mode
        {
            0xFFFF, //segment low
            0, //base low
            0, //base mid
            0b1010, // Executable 1 (Code segment) Direction 0 (Up) Readable 1 (True) Access 0 (default)
            1, // 1 meaning code/data
            0b00, // ring 0
            1, // 1 = valid
            0xF, //limit high
            0, // not for system software
            0, // Not 64 bit code
            0, // 16 bit protected mode
            0, // granularity 1
            0 //base high
        },
        {
            0xFFFF, //segment low
            0, //base low
            0, //base mid
            0b0010, // Executable 0 (Data segment) Direction 0 (Up) Readable 1 (True) Access 0 (default)
            1, // 1 meaning code/data
            0b00, // ring 0
            1, // 1 = valid
            0xF, //limit high
            0, // not for system software
            0, // Not 64 bit code
            0, // 16 bit protected mode
            0, // granularity 1
            0 //base high
        }
    }
};



/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table. 
    //        Use sizeof operator
    sizeof(global_descriptor_table)-1,
    &global_descriptor_table
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}

