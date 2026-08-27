; Intel (nasm)
; Header multiboot
section .multiboot
    align 4
    dd 0x1BADB002 ; MAGIC - dd for nasm == .long for gnu AT&T
    dd 0x3 ; FLAGS
    dd -(0x1BADB002 + 0x3) ; CHECKSUM

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

    