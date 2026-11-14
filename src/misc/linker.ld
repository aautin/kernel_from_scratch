ENTRY(kernel_entry)

PHDRS
{
	gdt    PT_LOAD FLAGS(4); /* Read-only GDT storage. */
	header PT_LOAD FLAGS(4); /* Read-only Multiboot header. */
	text   PT_LOAD FLAGS(5); /* Read and execute. */
	data   PT_LOAD FLAGS(6); /* Read and write. */
}

SECTIONS
{
	. = 0x800;
	.gdt :
	{
		KEEP(*(.gdt))
	} :gdt

	/*
		GRUB scans only the first 8192 file bytes for the Multiboot header.
		Keep its file position early while retaining its normal load address.
	*/
	.multiboot 1M : AT(1M)
	{
		/*
			multiboot section is never used in the program, only the bootloader search for it.
			We make sure the linker doesn't discard it, even if it is not referenced anywhere.
		*/
		KEEP(*(.multiboot))
	} :header

	.text 1M + 0x10 :
	{
		text_start = .;
		*(.text)
		text_end = .;
	} :text

	.rodata :
	{
		*(.rodata)
	} :text

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.data :
	{
		*(.data)
	} :data
	
	.bss (NOLOAD) :
	{
		/*
			We define the bss boundaries so we can clear it with zeros in the kernel, 
			so we can follow the C standards.
		*/

		bss_start = .;
		*(COMMON)
		*(.bss)
		*(.bss .bss.*)
		bss_end = .;
	} :data

	.stack (NOLOAD) :
	{
		. = ALIGN(16);
		stack_bottom = .;
		. = . + 0x4000;
		stack_top = .;
	} :data

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
