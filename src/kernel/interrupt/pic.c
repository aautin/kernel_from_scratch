#include "pic.h"
#include "port.h"

void remap_pic()
{
	//
	// ICW1
	//
	outb(PIC_MASTER_COMMAND, ICW1_BIT_ICW4 | ICW1_BIT_INITIALIZATION);
	io_wait();
	outb(PIC_SLAVE_COMMAND, ICW1_BIT_ICW4 | ICW1_BIT_INITIALIZATION);
	io_wait();
	
	//
	// ICW2
	// Master IRQs become vectors 0x20-0x27
	// Slave IRQs become vectors 0x28-0x2F
	//
	outb(PIC_MASTER_DATA, ICW2_MASTER_VECTOR_OFFSET);
	io_wait();
	outb(PIC_SLAVE_DATA, ICW2_SLAVE_VECTOR_OFFSET);
	io_wait();
	
	//
	// ICW3
	// Slave connected to master's IRQ2, and vice versa.
	//
	outb(PIC_MASTER_DATA, ICW3_SLAVE_CONNECTED_TO_IRQ2);
	io_wait();
	outb(PIC_SLAVE_DATA, ICW3_SLAVE_CONNECTED_TO_MASTER_IRQ2);
	io_wait();

	//
	// ICW4
	// Set the PICs to operate in 8086/88 (MCS-80/85) mode.
	//
	//
	outb(PIC_MASTER_DATA, ICW4_BIT_8086_MODE);
	io_wait();
	outb(PIC_SLAVE_DATA, ICW4_BIT_8086_MODE);
	io_wait();
	
	//
	// IMR
	// Enable IRQ1 (keyboard) and disable all other IRQs on both IPCs.
	//
	outb(PIC_MASTER_DATA,
		  IMR_BIT_DISABLE_IRQ0
		// IRQ1 doesn't get disabled
		| IMR_BIT_DISABLE_IRQ2
		| IMR_BIT_DISABLE_IRQ3
		| IMR_BIT_DISABLE_IRQ4
		| IMR_BIT_DISABLE_IRQ5
		| IMR_BIT_DISABLE_IRQ6
		| IMR_BIT_DISABLE_IRQ7
	);
	outb(PIC_SLAVE_DATA,
		  IMR_BIT_DISABLE_IRQ0
		| IMR_BIT_DISABLE_IRQ1
		| IMR_BIT_DISABLE_IRQ2
		| IMR_BIT_DISABLE_IRQ3
		| IMR_BIT_DISABLE_IRQ4
		| IMR_BIT_DISABLE_IRQ5
		| IMR_BIT_DISABLE_IRQ6
		| IMR_BIT_DISABLE_IRQ7
	);
}
