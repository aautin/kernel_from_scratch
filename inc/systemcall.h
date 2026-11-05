#pragma once

#include "interrupt.h"

enum syscall_number
{
	SYSCALL_GET_VERSION = 0,
};

enum syscall_result
{
	SYSCALL_ENOSYS = -38,
};

void syscall_dispatch(interrupt_frame_t* frame);
