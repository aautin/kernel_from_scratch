#include "multiboot.h"
#include "idt.h"
#include "gdt.h"
#include "pic.h"
#include "shell.h"
#include "terminal.h"

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

void kernel_main(multiboot_info_t* mbi)
{
	shell_init(mbi);
	terminal_init();

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
