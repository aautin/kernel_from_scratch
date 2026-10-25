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
// Like read(): starts a pending line read the first time it is called.
// While Enter is not pressed, it returns 0.
// After Enter, it copies the typed line (ending with '\n') into the buffer,
// then returns the number of copied characters.
//
uint32_t get_line(char* buffer, uint32_t size);
