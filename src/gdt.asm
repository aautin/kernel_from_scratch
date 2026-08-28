; gdt.asm

[bits 32]

section .text

	extern gdt_ptr
	global init_gdt

init_gdt:
	lgdt [gdt_ptr]

	; Reload CS with the 2nd (0x8) entry (kernel code segment)
	jmp 0x8:.reload_segments

.reload_segments:
	; 0x10 is the GDT table offset, it's the 3rd entry (kernel data segment)
	mov ax, 0x10

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret
