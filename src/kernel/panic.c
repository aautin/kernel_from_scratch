#include <stdint.h>
#include <stdbool.h>

#include "panic.h"
#include "cpu.h"
#include "idt.h"
#include "stack.h"
#include "printk.h"
#include "terminal.h"
#include "vga.h"

//
// The words of the stack shown by the panic screen, so the whole screen fits
// in 25 lines: 5 lines of exception and registers, 4 of word view header, the
// stack words, then 2 for the halt message.
//
#define PANIC_STACK_WORDS 14

static const char* exception_names[] =
{
	[INTERRUPT_VECTOR_DIVISION_BY_ZERO]              = "Division by Zero",
	[INTERRUPT_VECTOR_DEBUGGER]                      = "Debugger",
	[INTERRUPT_VECTOR_NMI]                           = "NMI",
	[INTERRUPT_VECTOR_BREAKPOINT]                    = "Breakpoint",
	[INTERRUPT_VECTOR_OVERFLOW]                      = "Overflow",
	[INTERRUPT_VECTOR_BOUNDS]                        = "Bounds",
	[INTERRUPT_VECTOR_INVALID_OPCODE]                = "Invalid Opcode",
	[INTERRUPT_VECTOR_COPROCESSOR_NOT_AVAILABLE]     = "Coprocessor not available",
	[INTERRUPT_VECTOR_DOUBLE_FAULT]                  = "Double fault",
	[INTERRUPT_VECTOR_COPROCESSOR_SEGMENT_OVERRUN]   = "Coprocessor Segment Overrun",
	[INTERRUPT_VECTOR_INVALID_TASK_STATE_SEGMENT]    = "Invalid Task State Segment",
	[INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT]           = "Segment not present",
	[INTERRUPT_VECTOR_STACK_FAULT]                   = "Stack Fault",
	[INTERRUPT_VECTOR_GENERAL_PROTECTION_FAULT]      = "General protection fault",
	[INTERRUPT_VECTOR_PAGE_FAULT]                    = "Page fault",
	[INTERRUPT_VECTOR_RESERVED]                      = "reserved",
	[INTERRUPT_VECTOR_MATH_FAULT]                    = "Math Fault",
	[INTERRUPT_VECTOR_ALIGNMENT_CHECK]               = "Alignment Check",
	[INTERRUPT_VECTOR_MACHINE_CHECK]                 = "Machine Check",
	[INTERRUPT_VECTOR_SIMD_FLOATING_POINT_EXCEPTION] = "SIMD Floating-Point Exception",
};

//
// The faults and the aborts: nothing in the kernel fixes the cause of these
// exceptions, so the CPU can't resume after them.
//
static const uint8_t fatal_exceptions[] =
{
	INTERRUPT_VECTOR_DIVISION_BY_ZERO,
	INTERRUPT_VECTOR_BOUNDS,
	INTERRUPT_VECTOR_INVALID_OPCODE,
	INTERRUPT_VECTOR_COPROCESSOR_NOT_AVAILABLE,
	INTERRUPT_VECTOR_DOUBLE_FAULT,
	INTERRUPT_VECTOR_COPROCESSOR_SEGMENT_OVERRUN,
	INTERRUPT_VECTOR_INVALID_TASK_STATE_SEGMENT,
	INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT,
	INTERRUPT_VECTOR_STACK_FAULT,
	INTERRUPT_VECTOR_GENERAL_PROTECTION_FAULT,
	INTERRUPT_VECTOR_PAGE_FAULT,
	INTERRUPT_VECTOR_MATH_FAULT,
	INTERRUPT_VECTOR_ALIGNMENT_CHECK,
	INTERRUPT_VECTOR_MACHINE_CHECK,
	INTERRUPT_VECTOR_SIMD_FLOATING_POINT_EXCEPTION,
};

//
// Static, so saving the stack doesn't use the stack.
//
static uint32_t saved_stack[PANIC_STACK_WORDS];
static bool     panicking = false;

static const char* exception_name(uint32_t vector)
{
	if (vector < sizeof(exception_names) / sizeof(exception_names[0]))
		return exception_names[vector];
	return "Unknown";
}

void panic_init()
{
	for (uint32_t i = 0; i < sizeof(fatal_exceptions) / sizeof(fatal_exceptions[0]); i++)
		interrupt_register(fatal_exceptions[i], panic);
}

void panic(interrupt_frame_t* frame)
{
	//
	// The CPU doesn't push SS:ESP for an exception in ring 0, so the
	// interrupted stack starts right after the frame. The 'esp' saved by
	// 'pusha' points inside the frame instead.
	//
	uint32_t esp = (uint32_t) (frame + 1);

	//
	// A fault while panicking stops the CPU right away
	//
	if (panicking)
		cpu_halt();
	panicking = true;

	//
	// Save the stack before anything else is printed or cleaned up
	//
	uint32_t saved_count = stack_copy(esp, saved_stack, PANIC_STACK_WORDS);

	terminal_clear();
	vga_set_cursor_visibility(false);

	printk("KERNEL PANIC: %s (vector 0x%02x, error 0x%08x)\n\n",
		exception_name(frame->vector), frame->vector, frame->error_code);
	printk("EAX=0x%08x  EBX=0x%08x  ECX=0x%08x  EDX=0x%08x\n",
		frame->eax, frame->ebx, frame->ecx, frame->edx);
	printk("ESI=0x%08x  EDI=0x%08x  EBP=0x%08x  ESP=0x%08x\n",
		frame->esi, frame->edi, frame->ebp, esp);
	printk("EIP=0x%08x  CS=0x%04x  EFLAGS=0x%08x\n",
		frame->eip, frame->cs, frame->eflags);

	stack_dump_words(saved_stack, saved_count, esp, frame->ebp);

	//
	// No newline after the last line, so the screen doesn't scroll
	//
	printk("\nSystem halted.");

	cpu_halt();
}
