#pragma once

//
// Stop the CPU for good: disable interrupts, clear the general purpose
// registers, then halt. Shared by the halt command and panic().
//
void cpu_halt() __attribute__((noreturn));
