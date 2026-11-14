#include <stdint.h>

#include "interrupt.h"
#include "idt.h"
#include "pic.h"
#include "port.h"

#define IDT_SIZE      256
#define PIC_IRQ_COUNT 16
#define PIC_EOI       0x20
#define EXCEPTION_COUNT 20

extern uint32_t exception_stub_table[EXCEPTION_COUNT];
extern uint32_t irq_stub_table[PIC_IRQ_COUNT];
extern void     syscall_stub();

static interrupt_handler_t handlers[IDT_SIZE];

void interrupt_init()
{
	for (uint32_t vector = 0; vector < EXCEPTION_COUNT; vector++)
	{
		set_interrupt_descriptor((enum interrupt_vector_index) vector, exception_stub_table[vector],
			INTERRUPT_FLAG_PRESENT | INTERRUPT_FLAG_DPL0 | INTERUPT_FLAG_32BIT_INTERRUPT_GATE);
	}

	for (uint32_t irq = 0; irq < PIC_IRQ_COUNT; irq++)
	{
		set_interrupt_descriptor((enum interrupt_vector_index) (PIC_OFFSET + irq), irq_stub_table[irq],
			INTERRUPT_FLAG_PRESENT | INTERRUPT_FLAG_DPL0 | INTERUPT_FLAG_32BIT_INTERRUPT_GATE);
	}

	set_interrupt_descriptor(INTERRUPT_VECTOR_SYSCALL, (uint32_t) syscall_stub,
		INTERRUPT_FLAG_PRESENT | INTERRUPT_FLAG_DPL3 | INTERUPT_FLAG_32BIT_INTERRUPT_GATE);
}

void interrupt_register(uint8_t vector, interrupt_handler_t handler)
{
	handlers[vector] = handler;
}

void interrupt_dispatch(interrupt_frame_t* frame)
{
	if (handlers[frame->vector])
		handlers[frame->vector](frame);

	// Only hardware IRQ vectors belong to the PIC. Syscalls and CPU exceptions
	// must not receive a PIC acknowledgement.
	if (frame->vector >= ICW2_MASTER_VECTOR_OFFSET
		&& frame->vector < ICW2_SLAVE_VECTOR_OFFSET + PIC_IRQ_COUNT / 2)
	{
		// Slave IRQs are routed through the master PIC, so acknowledge both.
		if (frame->vector >= ICW2_SLAVE_VECTOR_OFFSET)
			outb(PIC_SLAVE_COMMAND, PIC_EOI);

		// Master IRQs, including the cascade IRQ, are acknowledged here.
		outb(PIC_MASTER_COMMAND, PIC_EOI);
	}
}
