#include "multiboot.h"
#include "idt.h"
#include "terminal.h"
#include "gdt.h"
#include "pic.h"
#include "printk.h"

extern uint32_t irq1_stub();
extern void     register_gdt();

struct multiboot_mmap_entry
{
	uint32_t size;
	uint32_t base_addr_low;
	uint32_t base_addr_high;
	uint32_t length_low;
	uint32_t length_high;
	uint32_t type;
};

static void hang()
{
	//
	// Clear-interrupt-flag makes sure that no interrupts are received by the CPU
	// Halt puts the CPU in a sleep mode until an interrupt is received. The combination
	// of 'cli' and 'hlt' makes sure to stuck the CPU in sleep mode until shutdown/reboot
	//
	while (true)
	{
		__asm__ volatile ("cli");
		__asm__ volatile ("hlt");
	}
}

void kernel_main(struct multiboot_info* mbi)
{
	//
	// Disable interrupts during kernel initialization
	//
	__asm__ volatile ("cli");

	terminal_init();
	
	if (mbi->flags & MULTIBOOT_INFO_MEMORY)
	{
		printk("Lower memory: %d KB\n", mbi->mem_lower);
		printk("Upper memory: %d KB\n", mbi->mem_upper);
	}

	if (mbi->flags & MULTIBOOT_INFO_MEM_MAP)
	{
		printk("Memory map length: %d bytes\n", mbi->mmap_length);
		printk("Memory map address: 0x%x\n", mbi->mmap_addr);
		for (uint32_t i = 0; i < mbi->mmap_length; )
		{
			struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*) (mbi->mmap_addr + i);
			printk("Memory map entry: base_addr=0x%x, length=0x%x, type=%d\n", entry->base_addr_low, entry->length_low, entry->type);
			i += entry->size + sizeof(entry->size);
		}
	}

	set_gdt();
	register_gdt();
	
	remap_pic();

	set_interrupt_descriptor(INTERRUPT_VECTOR_KEYBOARD, (uint32_t) irq1_stub);
	register_interrupt_descriptor_table();

	//
	// Enable interrupts
	//
	__asm__ volatile ("sti");
	while (true)
	{
		//
		// Halt instruction to stop the CPU until the next interrupt occurs.
		// Kernel development common practice to prevent running idle loops.
		//
		__asm__ volatile ("hlt");
	}

	hang();
}
