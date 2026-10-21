#pragma once

#include <stdint.h>

//
// Print the stack from esp up to the top of the kernel stack: the used size,
// the possible return addresses, a hex/ASCII dump and a word view.
//
// esp and ebp are parameters so the same output can be reused, for example
// with the registers saved when the kernel panics.
//
void stack_dump(uint32_t esp, uint32_t ebp);
