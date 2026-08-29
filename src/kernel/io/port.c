#include <stdint.h>

void outb(uint16_t port, uint8_t value)
{
	__asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
	uint8_t value;
	__asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

void io_wait()
{
	//
	// 0x80 is used to be a "safe" port to read from to produce a small break
	// between two I/O operations, it's never used by hardwares to write to
	//
	outb(0x80, 0);
}
