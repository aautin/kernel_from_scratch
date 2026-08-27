#include <stdbool.h>

#include "multiboot.h"

static uint32_t write_index = 0;

static void write_debug_message(const char *message)
{
	volatile uint16_t *video_memory = (volatile uint16_t *) 0xB8000;

	for (uint32_t index = 0; message[index] != '\0'; index++)
	{
		if (write_index >= 80 * 25)
		{
			for (uint32_t i = 0; i < 80 * 25; i++)
			{
				video_memory[i] = (uint16_t) ' ' | 0x0F00;
			}

			write_index = 0;
		}

		if (message[index] == '\n')
		{
			write_index += 80 - (write_index % 80);
			continue;
		}

		video_memory[write_index] = (uint16_t) message[index] | 0x0F00;
		write_index++;
	}
}

void kernel_main(struct multiboot_info *mbi)
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
