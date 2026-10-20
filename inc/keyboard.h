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

//
// Like read(): waits until Enter is pressed, then copies the typed line,
// ending with '\n', into the buffer. Called by a command, which can print
// its own prompt before. Returns the number of characters copied.
//
uint32_t get_line(char* buffer, uint32_t size);
