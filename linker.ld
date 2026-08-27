ENTRY(start_kernel)

SECTIONS
{
	/*
		We discard the .eh_frame section, which is used for exception handling and stack unwinding.
		We also discard the .comment section, which contains version information about the compiler.
		We want the kernel to be as small as possible.
	*/

	/DISCARD/ :
	{ 
        *(.eh_frame) 
        *(.comment)
    }
	/* 
		The first 1 MiB of the physical address space contains many reserved/special regions.
		Kernels are conventionally loaded at 0x00100000 (1 MiB).
	*/
	. = 1M;

	.multiboot :
	{
		/*
			multiboot section is never used in the program, only the bootloader search for it.
			We make sure the linker doesn't discard it, even if it is not referenced anywhere.
		*/
		KEEP(*(.multiboot))
	}

	.rodata :
	{
		*(.rodata)
	}

	.data :
	{
		*(.data)
	}
	
	.text :
	{
		*(.text)
	}

	.bss :
	{
		/*
			bss is uninitialized data, the C standard requires it cleared with zeros at startup.
			We need to define the boundaries of bss section so we can clear it in the kernel.
		*/

		bss_start = .;
		*(COMMON)
		*(.bss)
		bss_end = .;
	}

	.stack :
	{
		stack_bottom = .;
		. = . + 0x4000;
		stack_top = .;
	}
}
