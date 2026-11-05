#include "systemcall.h"
#include "interrupt.h"

void syscall_dispatch(interrupt_frame_t* frame)
{
	// The ABI numbers are declared, but no syscall is implemented yet.
	frame->eax = (uint32_t) SYSCALL_ENOSYS;
}
