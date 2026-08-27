#include <stdbool.h>

#include "multiboot.h"

void kernel_main(struct multiboot_info *mbi)
{
	if (mbi->flags & MULTIBOOT_INFO_MEMORY)
	{
		uint32_t lower_memory = mbi->mem_lower;
		uint32_t upper_memory = mbi->mem_upper;

		(void) lower_memory;
		(void) upper_memory;

		//
		// TODO
		// Output a debug message
		//
	}

	if (mbi->flags & MULTIBOOT_INFO_MEM_MAP)
	{
		uint32_t mmap_length = mbi->mmap_length;
		uint32_t mmap_addr   = mbi->mmap_addr;

		(void) mmap_length;
		(void) mmap_addr;
		
		//
		// TODO
		// Output a debug message
		//
	}

	while (true)
	{
		//
		// Halt instruction to stop the CPU until the next interrupt occurs.
		// Kernel development common practice to prevent running idle loops and wasting power.
		//
		__asm__ volatile ("hlt");

		//
		// TODO
		// Handle interrupts from the keyboard
		//
	}
}
