; irq_stub.asm

[bits 32]

section .text

	MASTER_PIC_COMMAND_REGISTER equ 0x20
	END_OF_INTERRUPT_COMMAND    equ 0x20
	KEYBOARD_DATA_PORT          equ 0x60

	; irq1 is the keyboard interrupt.
	global irq1_stub

	extern keyboard_interrupt_handler

	irq1_stub:
		; Push the registers on the stack to save their state.
		pusha

		; Reads the keyboard scancode and passes it the handler 
		in al, KEYBOARD_DATA_PORT
		movzx eax, al
		push eax
		call keyboard_interrupt_handler

		; Move the stack pointer back to its original position.
		add esp, 4

		; out sends a byte from the CPU to an I/O port.
		; Hardware I/O ports are on a separate address space.
		mov al, END_OF_INTERRUPT_COMMAND
		out MASTER_PIC_COMMAND_REGISTER, al

		; Pop the registers from the stack to restore their state
		popa

		; Interrupt-return-doubleword.
		; Restores the CPU state (not the registers but the interrupt frame).
		iretd
