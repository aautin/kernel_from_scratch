#include "multiboot.h"
#include "idt.h"
#include "gdt.h"
#include "pic.h"
#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "mouse.h"
#include "interrupt.h"

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
	interrupt_init();
	interrupt_register(INTERRUPT_VECTOR_KEYBOARD, keyboard_interrupt_handler);
	interrupt_register(INTERRUPT_VECTOR_MOUSE, mouse_interrupt_handler);
	mouse_init();
	register_interrupt_descriptor_table();

	//
	// Enable interrupts
	//
	__asm__ volatile ("sti");
	while (true)
	{
		keyboard_handle_input();
		shell_update();

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

	hang();
}
