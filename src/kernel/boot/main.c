#include "multiboot.h"
#include "idt.h"
#include "terminal.h"
#include "gdt.h"
#include "pic.h"

extern uint32_t irq1_stub();
extern void     register_gdt();

static void hang()
{
	//
	// Clear-interrupt-flag makes sure that no interrupts are received by the CPU
	// Halt puts the CPU in a sleep mode until an interrupt is received. The combination
	// of 'cli' and 'hlt' makes sure to stuck the CPU in sleep mode until shutdown/reboot
	//
	while (true)
	{
		__asm__ volatile ("cli");
		__asm__ volatile ("hlt");
	}
}

void kernel_main(struct multiboot_info* mbi)
{
	//
	// Disable interrupts during kernel initialization
	//
	__asm__ volatile ("cli");

	terminal_init();
	terminal_puts("Kernel booted successfully\n");	

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
	}

	set_gdt();
	register_gdt();
	
	remap_pic();

	set_interrupt_descriptor(INTERRUPT_VECTOR_KEYBOARD, (uint32_t) irq1_stub);
	register_interrupt_descriptor_table();

	//
	// Enable interrupts
	//
	__asm__ volatile ("sti");
	while (true)
	{
		//
		// Halt instruction to stop the CPU until the next interrupt occurs.
		// Kernel development common practice to prevent running idle loops.
		//
		__asm__ volatile ("hlt");
	}

	hang();
}
