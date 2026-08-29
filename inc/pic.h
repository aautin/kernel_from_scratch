#pragma once

#include <stdint.h>

/*
	IRQ means Interrupt-Request.

	For a 32-bit x86 KFS, we're dealing with a 8259A-PIC devided into 2 parts.
	PIC Master : handles the IRQs from 0 to 7
	PIC Slave  : handles the IRQs from 8 to 15

	PIC Slave is connected to the PIC Master through IRQ2.

	We can communicate with the PICs through the following I/O ports:
	Master Command Port : 0x20
	Master Data Port    : 0x21
	Slave Command Port  : 0xA0
	Slave Data Port     : 0xA1

	Command and data ports leads to 2 different register groups.

	Ports are addresses but not in memory space.
	We use the inb/outb instructions to read/write to them.
*/

/*
	PIC communicates with the CPU through the IDT (Interrupt Descriptor Table).
	But this table is also used by the CPU to handle exceptions.
	CPU exceptions are mapped to the first 32 vectors (0x00-0x1F).
	And the PICs are also mapped to the first 16 vectors (0x00-0x0F).
	So we need to remap the PICs (offset of 32) to avoid conflicts with CPU.

	ICW = Initialization-Command-Words.
	IRM = Interrupt-Request-Mask.

	Remapping steps:
		ICW1 — start PIC initialization and indicate ICW4 will follow.
		ICW2 — choose the IDT vector bases (0x20 and 0x28).
		ICW3 — tell the master/slave how they're connected.
		ICW4 — select modes for EOI, buffering, nesting and architecture compliance.
		IMR  — decide which IRQ lines are enabled or masked.
*/

#define PIC_OFFSET 0x20

enum pic_port
{
	PIC_MASTER_COMMAND = 0x20,
	PIC_MASTER_DATA    = 0x21,
	PIC_SLAVE_COMMAND  = 0xA0,
	PIC_SLAVE_DATA     = 0xA1
};
typedef enum pic_port pic_port_t;

enum pic_icw1
{
	//
	// Set it if ICW4 is needed.
	//
	ICW1_BIT_ICW4 = 1 << 0,

	//
	// Set it if the PIC isn't-cascaded (no master/slave configuration).
	//
	ICW1_BIT_CASCADE = 1 << 1,

	//
	// Set it if PICs operating on a table (IDT) of 4-bytes sized elements.
	// The default is 8-bytes sized.
	//
	ICW1_BIT_INTERVAL = 1 << 2,

	//
	// Set it if the PIC is level triggered, otherwise its edge triggered.
	// Level: PICs keep sending an interrupt until CPU acknowledges it.
	// Edge: PICs send an interrupt only once.
	//
	ICW1_BIT_LEVEL_TRIGGERED = 1 << 3,

	//
	// Set it to indicate this command is an initialization command.
	// Don't set it for a simple command.
	//
	ICW1_BIT_INITIALIZATION = 1 << 4
};

enum pic_icw2
{
	ICW2_MASTER_VECTOR_OFFSET = PIC_OFFSET,
	ICW2_SLAVE_VECTOR_OFFSET  = PIC_OFFSET + 8
};

enum pic_icw3
{
	ICW3_SLAVE_CONNECTED_TO_IRQ0 = 1 << 0,
	ICW3_SLAVE_CONNECTED_TO_IRQ1 = 1 << 1,
	ICW3_SLAVE_CONNECTED_TO_IRQ2 = 1 << 2,
	ICW3_SLAVE_CONNECTED_TO_IRQ3 = 1 << 3,
	ICW3_SLAVE_CONNECTED_TO_IRQ4 = 1 << 4,
	ICW3_SLAVE_CONNECTED_TO_IRQ5 = 1 << 5,
	ICW3_SLAVE_CONNECTED_TO_IRQ6 = 1 << 6,
	ICW3_SLAVE_CONNECTED_TO_IRQ7 = 1 << 7,

	ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ0 = 1 << 0,
	ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ1 = 1 << 1,
	ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ2 = 1 << 2,
	ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ3 = 1 << 3,
	ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ4 = 1 << 4,
	ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ5 = 1 << 5,
	ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ6 = 1 << 6,
	ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ7 = 1 << 7,
};

enum pic_icw4
{
	//
	// Per default, PICs operate in 8080/8085 mode.
	// Set this bit to make them operate in 8086/88 mode.
	//
	ICW4_BIT_8086_MODE = 1 << 0,

	//
	// Set it if the PIC should automatically send an EOI (End Of Interrupt)
	// signal to the CPU after each interrupt.
	//
	ICW4_BIT_AUTO_EOI  = 1 << 1,

	//
	// Ignored for our purposes.
	//
	ICW4_BIT_BUFFERED_MASTER = 1 << 2,
	ICW4_BIT_BUFFERED_SLAVE  = 1 << 3,
	ICW4_BIT_SFNM      = 1 << 4
};

enum pic_imr
{
	IMR_BIT_DISABLE_IRQ0  = 1 << 0,
	IMR_BIT_DISABLE_IRQ1  = 1 << 1,
	IMR_BIT_DISABLE_IRQ2  = 1 << 2,
	IMR_BIT_DISABLE_IRQ3  = 1 << 3,
	IMR_BIT_DISABLE_IRQ4  = 1 << 4,
	IMR_BIT_DISABLE_IRQ5  = 1 << 5,
	IMR_BIT_DISABLE_IRQ6  = 1 << 6,
	IMR_BIT_DISABLE_IRQ7  = 1 << 7
};

void remap_pic();
