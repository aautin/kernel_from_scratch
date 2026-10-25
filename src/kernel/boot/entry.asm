; boot.asm

[bits 32]

section .text

	extern kernel_main
	extern bss_start
	extern bss_end
	extern stack_top

	global kernel_entry

	kernel_entry:
		; Disable interrupts during the boot process
		cli


		; Point ESP to the top of the Kernel stack
		mov esp, stack_top


		; Zero the .bss section
		mov edi, bss_start
		mov ecx, bss_end
		sub ecx, edi       ; put in ecx the number of bytes between start and end
		xor eax, eax       ; set eax at 0 (including al, its 8 bits register)
		cld                ; clear direction flag (0 = forward, 1 = backward)
		rep stosb          ; Store String Byte =
		                   ; - put al at address of edi
						   ; - ecx is used as the number of times it goes in a direction
						   ; - repeat


		; Align the stack on 16 bytes before the call
		sub esp, 12


		; Push the multiboot information structure pointer onto the stack
		push ebx


		; Call the kernel main function
		call kernel_main
