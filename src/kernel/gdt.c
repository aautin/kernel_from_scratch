#include "gdt.h"

static gdt_entry_t gdt[5];
gdt_ptr_t   gdt_ptr;

void register_gdt()
{
	gdt_ptr.limit = sizeof(gdt) - 1;
	gdt_ptr.base  = (uint32_t) &gdt;

	asm volatile("lgdt %0" : : "m" (gdt_ptr));
}

void set_gdt()
{
	//
	// Null segment
	//
	set_gdt_entry(NULL_I,
		ADDRESS_MIN, ADDRESS_MIN,
		0, 0);

	//
	// Kernel code segment
	//
	set_gdt_entry(KERNEL_CODE_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_0 | EXECUTABLE | READ_WRITE, 0xCF
	);
	
	//
	// Kernel data segment
	//
	set_gdt_entry(KERNEL_DATA_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_0 | READ_WRITE, 0xCF
	);

	//
	// User code segment
	//
	set_gdt_entry(USER_CODE_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_3 | EXECUTABLE | READ_WRITE, 0xCF
	);

	//
	// User data segment
	//
	set_gdt_entry(USER_DATA_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_3 | READ_WRITE, 0xCF
	);
}

void set_gdt_entry(selector_index index, uint32_t base, uint32_t limit,
					uint8_t access, uint8_t granularity)
{
	gdt[index].base_low    = (base & 0xFFFF);
	gdt[index].base_middle = (base >> 16) & 0xFF;
	gdt[index].base_high   = (base >> 24) & 0xFF;

	gdt[index].limit_low   = (limit & 0xFFFF);
	gdt[index].granularity = ((limit >> 16) & 0x0F);

	gdt[index].granularity |= (granularity & 0xF0);
	gdt[index].access      = access;
}