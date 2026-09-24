; gdt.asm

[bits 32]

section .gdt

	global gdt_start
	global gdt_end

	gdt_start:
		; Reserve 7 quadwords.
		; For 7 descriptors: null, kernel code-data-stack, user code-data-stack
		times 7 dq 0
	gdt_end:
