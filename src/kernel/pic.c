#include "pic.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void io_wait()
{
    outb(0x80, 0);
}

void remap_pic()
{

	outb(PIC_MASTER_COMMAND, 0x11);
    io_wait();
	outb(PIC_SLAVE_COMMAND, 0x11);
    io_wait();

	//
	// TODO
	// Improve remap process understanding
	//
	
	//
	// Master IRQs become vectors 0x20-0x27
	// the first 32 vectors are reserved for CPU exceptions
	//
	outb(PIC_MASTER_MASK, REMAP_OFFSET_MASTER);
    io_wait();
	//
	// Slave IRQs become vectors 0x28-0x2F
	//
	outb(PIC_SLAVE_MASK, REMAP_OFFSET_SLAVE);
    io_wait();
	
	//
	// Master has slave connected to IRQ2
	//
	outb(PIC_MASTER_MASK, PIC_MASTER_MASK_SLAVE);
    io_wait();
	//
	// Slave is connected to master's IRQ2
	//
	outb(PIC_SLAVE_MASK, PIC_SLAVE_MASK_IRQ2);
    io_wait();

	//
	// Set the PICs to operate in 8086/88 (MCS-80/85) mode.
	//
	outb(PIC_MASTER_MASK, 0x01);
    io_wait();
	outb(PIC_SLAVE_MASK, 0x01);
    io_wait();
	
    // Restore masks, but enable keyboard IRQ1.
    outb(0x21, 0xFD); // Enable IRQ1, disable other master IRQs
	outb(0xA1, 0xFF); // Disable all slave IRQs
}
