#include <stdint.h>

#include "interrupt.h"
#include "idt.h"
#include "pic.h"
#include "port.h"

#define IDT_SIZE      256
#define PIC_IRQ_COUNT 16
#define PIC_EOI       0x20

extern uint32_t irq_stub_table[PIC_IRQ_COUNT];

static interrupt_handler_t handlers[IDT_SIZE];

void interrupt_init()
{
	for (uint32_t irq = 0; irq < PIC_IRQ_COUNT; irq++)
		set_interrupt_descriptor((enum interrupt_vector_index) (PIC_OFFSET + irq), irq_stub_table[irq]);
}

void interrupt_register(uint8_t vector, interrupt_handler_t handler)
{
	handlers[vector] = handler;
}

void interrupt_dispatch(interrupt_frame_t* frame)
{
	if (handlers[frame->vector])
		handlers[frame->vector](frame);

	//
	// The slave PIC is behind the master one, so its IRQs are acknowledged
	// to both PICs
	//
	if (frame->vector >= ICW2_SLAVE_VECTOR_OFFSET)
		outb(PIC_SLAVE_COMMAND, PIC_EOI);
	outb(PIC_MASTER_COMMAND, PIC_EOI);
}
