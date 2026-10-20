#pragma once

#include "interrupt.h"

//
// Enable the mouse on the PS/2 controller, with the scroll wheel when the
// mouse has one. Called while interrupts are still disabled.
//
void mouse_init();

//
// Turns the scroll wheel into Up/Down arrow keys, so it scrolls the terminal.
//
void mouse_interrupt_handler(interrupt_frame_t* frame);
