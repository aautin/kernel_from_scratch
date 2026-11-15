#include <stdint.h>

#include "timer.h"
#include "port.h"

//
// The PIT input clock, divided by the reload value of a channel to get its
// output frequency.
//
#define PIT_BASE_FREQUENCY 1193182

enum pit_port
{
	PIT_CHANNEL0_DATA = 0x40,
	PIT_COMMAND       = 0x43,
};

enum pit_command
{
	PIT_SELECT_CHANNEL0 = 0b00 << 6,
	PIT_ACCESS_LOW_HIGH = 0b11 << 4, // The reload value is sent low byte first
	PIT_MODE_RATE       = 0b010 << 1, // Mode 2: one pulse every reload value
	PIT_BINARY          = 0,
};

static volatile uint32_t ticks = 0;

void timer_init()
{
	uint16_t divisor = (PIT_BASE_FREQUENCY + TIMER_FREQUENCY / 2) / TIMER_FREQUENCY;

	outb(PIT_COMMAND, PIT_SELECT_CHANNEL0 | PIT_ACCESS_LOW_HIGH | PIT_MODE_RATE | PIT_BINARY);
	outb(PIT_CHANNEL0_DATA, (uint8_t) (divisor & 0xFF));
	outb(PIT_CHANNEL0_DATA, (uint8_t) (divisor >> 8));
}

void timer_interrupt_handler(interrupt_frame_t* frame)
{
	(void) frame;
	ticks++;
}

uint32_t timer_ticks()
{
	return ticks;
}
