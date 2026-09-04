#include "gdt.h"
#include "printk.h"

extern gdt_entry_t gdt_start[];
extern gdt_entry_t gdt_end[];

gdt_entry_t* gdt     = gdt_start;
gdt_ptr_t    gdt_ptr = { 0, 0 };

static void set_entry(selector_i index, uint32_t base, uint32_t limit,
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

void register_gdt()
{
    gdt_ptr.limit = (uint32_t)gdt_end - (uint32_t)gdt_start - 1;
    gdt_ptr.base  = (uint32_t)gdt;

	//
	// Null segment
	//
	set_entry(NULL_I,
		ADDRESS_MIN, ADDRESS_MIN,
		0, 0
	);

	//
	// Kernel code segment
	//
	set_entry(KERNEL_CODE_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_0 | EXECUTABLE | READ_WRITE, 0xCF
	);
	
	//
	// Kernel data segment
	//
	set_entry(KERNEL_DATA_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_0 | READ_WRITE, 0xCF
	);

	//
	// Kernel stack segment
	//
	set_entry(KERNEL_STACK_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_0 | READ_WRITE, 0xCF
	);

	//
	// User code segment
	//
	set_entry(USER_CODE_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_3 | EXECUTABLE | READ_WRITE, 0xCF
	);

	//
	// User data segment
	//
	set_entry(USER_DATA_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_3 | READ_WRITE, 0xCF
	);

	//
	// User stack segment
	//
	set_entry(USER_STACK_I,
		ADDRESS_MIN, ADDRESS_MAX,
		DESCRIPTOR | PRESENT | PRIVILEGE_3 | READ_WRITE, 0xCF
	);

	//
	// Load the GDT register with the address of the GDT pointer
	//
	__asm__ volatile ("lgdt %0" : : "m"(gdt_ptr));

	//
	// Reload the segment registers with the new GDT values:
	// CS uses the KERNEL_CODE segment selector
	// DS, ES, FS and GS use the KERNEL_DATA segment selector
	// SS uses the KERNEL_STACK segment selector
	//
	__asm__ volatile (
		"ljmp %0, $reload_segments\n"
		"reload_segments:\n"

		"mov %1, %%ax\n"
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"

		"mov %2, %%ax\n"
		"mov %%ax, %%ss\n"

		//
		// Output operands 
		//
		:
		//
		// Input operands. Tells GCC what values to substitute into the asm code.
		//
		: "i"(KERNEL_CODE_OFFSET),
		  "i"(KERNEL_DATA_OFFSET),
		  "i"(KERNEL_STACK_OFFSET)
		//
		// Clobber list. Tells GCC what this asm modifies/affects.
		//
		// EAX is used to load the segment registers
		// and memory is affected by the segment register changes.
		//
		: "eax", "memory"
	);
}
