; irq_stub.asm

[bits 32]

section .text

	PIC_OFFSET equ 0x20

	extern interrupt_dispatch

	; One stub per IRQ: it pushes a dummy error code and the vector number, so
	; every interrupt reaches the common stub with the same stack layout.
	%macro IRQ_STUB 1
	global irq%1_stub
	irq%1_stub:
		push 0
		push PIC_OFFSET + %1
		jmp interrupt_common_stub
	%endmacro

	IRQ_STUB 0
	IRQ_STUB 1
	IRQ_STUB 2
	IRQ_STUB 3
	IRQ_STUB 4
	IRQ_STUB 5
	IRQ_STUB 6
	IRQ_STUB 7
	IRQ_STUB 8
	IRQ_STUB 9
	IRQ_STUB 10
	IRQ_STUB 11
	IRQ_STUB 12
	IRQ_STUB 13
	IRQ_STUB 14
	IRQ_STUB 15

	interrupt_common_stub:
		; Push the registers on the stack to save their state.
		pusha

		; The stack now holds an interrupt_frame_t (see interrupt.h),
		; its address is passed to the dispatcher.
		push esp
		call interrupt_dispatch

		; Move the stack pointer back to its original position.
		add esp, 4

		; Pop the registers from the stack to restore their state
		popa

		; Remove the vector number and the error code.
		add esp, 8

		; Interrupt-return-doubleword.
		; Restores the CPU state (not the registers but the interrupt frame).
		iretd

section .rodata

	; Addresses of the stubs, indexed by IRQ number.
	global irq_stub_table
	irq_stub_table:
	%assign i 0
	%rep 16
		dd irq %+ i %+ _stub
	%assign i i + 1
	%endrep
