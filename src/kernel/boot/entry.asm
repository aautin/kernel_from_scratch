; boot.asm

[bits 32]

section .text

	extern kernel_main
	global kernel_entry

	kernel_entry:
		; Disable interrupts during the boot process
		cli

		; Push the multiboot information structure pointer onto the stack
		push ebx

		; Call the kernel main function
		call kernel_main
