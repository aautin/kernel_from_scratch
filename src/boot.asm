; boot.asm

[bits 32]

section .multiboot

	MULTIBOOT_PAGE_ALIGN     equ 0x00000001
	MULTIBOOT_MEMORY_INFO    equ 0x00000002
	MULTIBOOT_VIDEO_MODE     equ 0x00000004
	MULTIBOOT_ADDRESS_FIELDS equ 0x00010000

	MULTIBOOT_MAGIC            equ 0x1BADB002
	MULTIBOOT_FLAGS            equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
	MULTIBOOT_CHECKSUM         equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

	; make the multiboot aligned on 32-bits
	align 4
	dd MULTIBOOT_MAGIC
	dd MULTIBOOT_FLAGS
	dd MULTIBOOT_CHECKSUM

section .text
	
	global start_kernel

	; stack_top is defined in the linker script (linker.ld)
	; kernel_main is a function in kernel/main.c
	extern stack_top
	extern kernel_main

	start_kernel:
		; Move the stack pointer on the stack top, the stack should grow downwards only until the __stack_bottom symbol
		mov esp, stack_top

		; Clear-interrupt-flag makes sure that no interrupts are received while the kernel is being initialized
		cli

		; Push the multiboot information structure pointer stored in ebx by the bootloader
		push ebx
		call kernel_main

	.hang:
		; Clear-interrupt-flag makes sure that no interrupts are received by the CPU
		; Halt makes sure the CPU is put in a sleep mode until an interrupt is received
		; The combination of cli and hlt makes sure to stuck the CPU in a sleep mode until a reset is received
		cli
		hlt

		; Loop forever as a last resort, we should never reach this point
		jmp .hang
