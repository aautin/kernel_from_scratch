; exception_stub.asm

[bits 32]

section .text

	extern interrupt_common_stub

	; One stub per CPU exception, so every exception reaches the common stub
	; with the same stack layout as the IRQs (see interrupt.h).

	; The CPU doesn't push an error code: push a dummy one.
	%macro EXCEPTION_STUB 1
	global exception%1_stub
	exception%1_stub:
		push 0
		push %1
		jmp interrupt_common_stub
	%endmacro

	; The CPU already pushed an error code: only push the vector number.
	%macro EXCEPTION_STUB_ERROR_CODE 1
	global exception%1_stub
	exception%1_stub:
		push %1
		jmp interrupt_common_stub
	%endmacro

	EXCEPTION_STUB            0  ; Division by Zero
	EXCEPTION_STUB            1  ; Debugger
	EXCEPTION_STUB            2  ; NMI
	EXCEPTION_STUB            3  ; Breakpoint
	EXCEPTION_STUB            4  ; Overflow
	EXCEPTION_STUB            5  ; Bounds
	EXCEPTION_STUB            6  ; Invalid Opcode
	EXCEPTION_STUB            7  ; Coprocessor not available
	EXCEPTION_STUB_ERROR_CODE 8  ; Double fault (error code is always 0)
	EXCEPTION_STUB            9  ; Coprocessor Segment Overrun (386 or earlier only)
	EXCEPTION_STUB_ERROR_CODE 10 ; Invalid Task State Segment
	EXCEPTION_STUB_ERROR_CODE 11 ; Segment not present
	EXCEPTION_STUB_ERROR_CODE 12 ; Stack Fault
	EXCEPTION_STUB_ERROR_CODE 13 ; General protection fault
	EXCEPTION_STUB_ERROR_CODE 14 ; Page fault
	EXCEPTION_STUB            15 ; reserved
	EXCEPTION_STUB            16 ; Math Fault
	EXCEPTION_STUB_ERROR_CODE 17 ; Alignment Check
	EXCEPTION_STUB            18 ; Machine Check
	EXCEPTION_STUB            19 ; SIMD Floating-Point Exception

section .rodata

	; Addresses of the stubs, indexed by exception vector.
	global exception_stub_table
	exception_stub_table:
	%assign i 0
	%rep 20
		dd exception %+ i %+ _stub
	%assign i i + 1
	%endrep
