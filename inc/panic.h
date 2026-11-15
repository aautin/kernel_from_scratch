#pragma once

#include "interrupt.h"

//
// Register panic() as the handler of the fatal CPU exceptions (the faults and
// the aborts).
//
void panic_init();

//
// Handler for the fatal CPU exceptions: save the interrupted stack, print the
// exception, the registers and the saved stack on one screen, then stop the
// CPU with cpu_halt(). Never returns to the faulting instruction.
//
void panic(interrupt_frame_t* frame) __attribute__((noreturn));
