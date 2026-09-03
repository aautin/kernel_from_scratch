; Intel (nasm)
; Header multiboot
section .multiboot
    align 4
    dd 0x1BADB002 ; MAGIC - dd for nasm == .long for gnu AT&T
    dd 0x3 ; FLAGS
    dd -(0x1BADB002 + 0x3) ; CHECKSUM

section .gdt
    global gdt_start
    global gdt_end
    global gdt_flush

    gdt_start:
        resq 6
    gdt_end:

    gdt_flush:
        jmp 0x08:.reload_cs
    .reload_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

section .bss ; provides a stack size and define the stack_top
    align 16
    stack_bottom:
        resb 16384 ; 16 KiB de stack
    stack_top:

; code
section .text
    global _start
    extern kernel_main

    _start:
        mov esp, stack_top
        call kernel_main
    halt:
        hlt
        jmp halt

    