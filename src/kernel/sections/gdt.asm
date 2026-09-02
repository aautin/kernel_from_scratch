; gdt.asm

[bits 32]

section .gdt

	global gdt_start
	global gdt_end

	gdt_start:
		; Reserve 5 quadwords.
		; 5 descriptors: null, kernel code, kernel data, user code, user data
		resq 5      
	gdt_end:
