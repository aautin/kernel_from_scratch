#include "multiboot.h"
#include "idt.h"
#include "gdt.h"
#include "pic.h"
#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "mouse.h"
#include "interrupt.h"
#include "panic.h"
#include "cpu.h"

extern void register_gdt();

void kernel_main(multiboot_info_t* mbi)
{
	shell_init(mbi);
	terminal_init();

	register_gdt();
	remap_pic();
	interrupt_init();
	interrupt_register(INTERRUPT_VECTOR_KEYBOARD, keyboard_interrupt_handler);
	interrupt_register(INTERRUPT_VECTOR_MOUSE, mouse_interrupt_handler);
	panic_init();
	mouse_init();
	register_interrupt_descriptor_table();	

	//
	// Enable interrupts
	//
	__asm__ volatile ("sti");
	while (true)
	{
		keyboard_handle_input();

		//
		// Halt instruction to stop the CPU until the next interrupt occurs.
		// Kernel development common practice to prevent running idle loops.
		//
		// Interrupts are disabled while checking the key buffer, so a key
		// received between the check and 'hlt' can't be missed: 'sti' only
		// enables interrupts after the next instruction, here 'hlt'.
		//
		__asm__ volatile ("cli");
		if (!keyboard_has_input())
			__asm__ volatile ("sti; hlt");
		__asm__ volatile ("sti");
	}

	cpu_halt();
}
