; boot.asm

[bits 32]

section .text

	extern kernel_main
	extern stack_top ; defined by the linker
	global kernel_entry

	kernel_entry:
		; Disable interrupts during the boot process
		cli

		; Point ESP to the top of the Kernel stack
		mov esp, stack_top

		; Align the stack on 16 bytes before the call
		sub esp, 12

		; Push the multiboot information structure pointer onto the stack
		push ebx


		; Call the kernel main function
		call kernel_main
