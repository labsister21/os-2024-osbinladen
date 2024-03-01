#include "header/cpu/gdt.h"

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