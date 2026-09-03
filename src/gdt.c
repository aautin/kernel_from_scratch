#include "gdt.h"

extern gdt_entry_t gdt_start[];
extern gdt_entry_t gdt_end[];



void set_entry(segment index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
{
    gdt_start[index].base_low = (base & 0xFFFF);
    gdt_start[index].base_middle = (base >> 16) & 0xFF;
    gdt_start[index].base_high = (base >> 24) & 0xFF;

    gdt_start[index].limit_low = (limit & 0xFFFF);
    gdt_start[index].granularity = ((limit >> 16) & 0x0F);

    gdt_start[index].granularity |= (granularity & 0xF0);
    gdt_start[index].access = access;
}

void register_gdt(void)
{
    gdt_ptr_t gdt_ptr;
    gdt_ptr.limit = (uint32_t)gdt_end - (uint32_t)gdt_start - 1;
    gdt_ptr.base = (uint32_t)&gdt_start;


    set_entry(NULL_I, 0, 0, 0, 0); // Null segment
    set_entry(KERNEL_CODE_I, 0, 0xFFFFFFFF, DESCRIPTOR | PRESENT | PRIVILEGE_0 | EXECUTABLE | READ_WRITE, 0xCF);
    set_entry(KERNEL_DATA_I, 0, 0xFFFFFFFF, DESCRIPTOR | PRESENT | PRIVILEGE_0 | READ_WRITE, 0xCF);
    set_entry(USER_CODE_I, 0, 0xFFFFFFFF, DESCRIPTOR | PRESENT | PRIVILEGE_3 | EXECUTABLE | READ_WRITE, 0xCF);
    set_entry(USER_DATA_I, 0, 0xFFFFFFFF, DESCRIPTOR | PRESENT | PRIVILEGE_3 | READ_WRITE, 0xCF);

    // Load the GDT
    asm volatile("lgdt %0" : : "m"(gdt_ptr));

    // Flush the old segment registers
    gdt_flush();
}