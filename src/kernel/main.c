#include <stdbool.h>

#include "multiboot.h"
#include "interrupt.h"
#include "vga.h"
#include "gdt.h"
#include "pic.h"

extern uint32_t irq1_stub();
extern void     init_gdt();

void kernel_main(struct multiboot_info* mbi)
{
	write_debug_message("Kernel booted successfully\n");
	
	if (mbi->flags & MULTIBOOT_INFO_MEMORY)
	{
		uint32_t lower_memory = mbi->mem_lower;
		uint32_t upper_memory = mbi->mem_upper;

		(void) lower_memory;
		(void) upper_memory;
		//
		// TODO
		// Output the lower and upper memory information to the debug console
		//

		write_debug_message("MULTIBOOT_INFO_MEMORY activated\n");
	}

	if (mbi->flags & MULTIBOOT_INFO_MEM_MAP)
	{
		uint32_t mmap_length = mbi->mmap_length;
		uint32_t mmap_addr   = mbi->mmap_addr;

		(void) mmap_length;
		(void) mmap_addr;
		//
		// TODO
		// Output the memory map information to the debug console
		//

		write_debug_message("MULTIBOOT_INFO_MEM_MAP activated\n");
	}

	set_gdt();
	register_gdt();
	init_gdt();

	remap_pic();

	set_interrupt_descriptor(INTERRUPT_VECTOR_KEYBOARD, (uint32_t) irq1_stub);
	register_interrupt_descriptor_table();

	__asm__ volatile ("sti");
	
	while (true)
	{

		//
		// Halt instruction to stop the CPU until the next interrupt occurs.
		// Kernel development common practice to prevent running idle loops and wasting power.
		//
		__asm__ volatile ("hlt");

		write_debug_message("*");
	}
}
