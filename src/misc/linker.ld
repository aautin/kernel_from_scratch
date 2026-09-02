ENTRY(kernel_entry)

SECTIONS
{
	. = 0x800;
	.gdt :
	{
		KEEP(*(.gdt))
	}

	. = 1M;
	.multiboot :
	{
		/*
			multiboot section is never used in the program, only the bootloader search for it.
			We make sure the linker doesn't discard it, even if it is not referenced anywhere.
		*/
		KEEP(*(.multiboot))
	}

	.text :
	{
		*(.text)
	}

	.rodata :
	{
		*(.rodata)
	}

	.data :
	{
		*(.data)
	}
	
	.bss (NOLOAD) :
	{
		/*
			We define the bss boundaries so we can clear it with zeros in the kernel, 
			so we can follow the C standards.
		*/

		bss_start = .;
		*(COMMON)
		*(.bss)
		bss_end = .;
	}

	.stack (NOLOAD) :
	{
		stack_bottom = .;
		. = . + 0x4000;
		stack_top = .;
	}

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
}
