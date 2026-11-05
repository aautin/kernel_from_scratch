#include <stdint.h>

#include "systemcall.h"
#include "interrupt.h"

#define KERNEL_SYSCALL_VERSION 1

void syscall_dispatch(interrupt_frame_t* frame)
{
	switch (frame->eax)
	{
		case SYSCALL_GET_VERSION:
			frame->eax = KERNEL_SYSCALL_VERSION;
			break;
		default:
			//
			// ENOSYS means “functionality not implemented” or “invalid/unimplemented system call.”
			// In this kernel version, we haven't implemented the handling, only the stub and dispatching parts.
			//
			frame->eax = (uint32_t) SYSCALL_ENOSYS;
			break;
	}
}
