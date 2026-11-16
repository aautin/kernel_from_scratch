#include "exception.h"
#include "idt.h"
#include "interrupt.h"
#include "signal.h"

static void breakpoint_handler(interrupt_frame_t* frame)
{
	(void) frame;
	(void) signal_raise(SIGNAL_TRAP);
}

static void overflow_handler(interrupt_frame_t* frame)
{
	(void) frame;
	(void) signal_raise(SIGNAL_SEGMENTATION_FAULT);
}

void exception_init()
{
	interrupt_register(INTERRUPT_VECTOR_BREAKPOINT, breakpoint_handler);
	interrupt_register(INTERRUPT_VECTOR_OVERFLOW, overflow_handler);
}
