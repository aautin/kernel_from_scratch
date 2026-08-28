#include "interrupt.h"
#include "gdt.h"
#include "vga.h"

static idt_t     idt[256];
static idt_ptr_t idt_ptr;

void register_interrupt_descriptor_table()
{
	idt_ptr.limit = sizeof(idt) - 1;
	idt_ptr.base  = (uint32_t) &idt;

	asm volatile("lidt %0" : : "m" (idt_ptr));
}

void set_interrupt_descriptor(enum interrupt_vector_index index, uint32_t handler)
{
	idt[index].base_low  = (uint16_t) (handler & 0xFFFF);
	idt[index].selector  = (selector_offset) KERNEL_CODE_OFFSET;
	idt[index].zero      = 0;
	idt[index].flags     = 0x8E; // Present, ring 0, ring 3 would be 0xEE, 32-bit interrupt gate
	idt[index].base_high = (uint16_t) ((handler >> 16) & 0xFFFF);
}

void keyboard_interrupt_handler(uint32_t interrupt_number)
{
	(void) interrupt_number;
	
	write_debug_message("Keyboard interrupt received\n");
}
