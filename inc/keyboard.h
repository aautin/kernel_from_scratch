#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"

void keyboard_interrupt_handler(interrupt_frame_t* frame);

//
// Add a scancode to the key buffer, as if the key was pressed.
//
void keyboard_push_scancode(uint8_t scancode);

//
// Called by the main loop: handles the stored scancodes, and runs the shell
// commands outside of the interrupt.
//
bool keyboard_has_input();
void keyboard_handle_input();
