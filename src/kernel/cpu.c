#include "cpu.h"

void cpu_halt()
{
	//
	// Clear-interrupt-flag makes sure that no interrupts are received by the CPU,
	// so keyboard input is no longer handled.
	// Clear the general purpose registers, then halt again after any
	// non-maskable interrupt wakes the CPU.
	//
	__asm__ volatile (
		"cli\n"
		"xor %eax, %eax\n"
		"xor %ebx, %ebx\n"
		"xor %ecx, %ecx\n"
		"xor %edx, %edx\n"
		"xor %esi, %esi\n"
		"xor %edi, %edi\n"
		"xor %ebp, %ebp\n"
		"1:\n"
		"cli\n"
		"hlt\n"
		"jmp 1b\n"
	);
	__builtin_unreachable();
}
