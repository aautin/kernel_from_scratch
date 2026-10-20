#pragma once

#include <stdint.h>

//
// What the stack holds when the dispatcher is called, from the last value
// pushed to the first one:
// - the registers saved by 'pusha' in irq_stub.asm,
// - the vector number and the error code pushed by the IRQ stub,
// - the values pushed by the CPU itself before calling the stub.
//
struct interrupt_frame
{
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	uint32_t vector;
	uint32_t error_code;

	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
} __attribute__((packed));
typedef struct interrupt_frame interrupt_frame_t;

typedef void (*interrupt_handler_t)(interrupt_frame_t* frame);

//
// Point the IDT entries of the 16 IRQs to their stubs, which all call
// interrupt_dispatch().
//
void interrupt_init();

//
// Set the function called when the interrupt of the given vector occurs.
//
void interrupt_register(uint8_t vector, interrupt_handler_t handler);

//
// Called by the IRQ stubs: calls the registered handler, then acknowledges
// the interrupt to the PIC.
//
void interrupt_dispatch(interrupt_frame_t* frame);
