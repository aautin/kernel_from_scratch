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

//
// Copy at most count words of the kernel stack, from esp up to the top of the
// stack, into buffer. Returns the number of words copied, 0 if esp is not in
// the kernel stack.
//
uint32_t stack_copy(uint32_t esp, uint32_t* buffer, uint32_t count);

//
// Print the word view of stack_dump() for count words copied from the stack
// at esp, so a saved copy is shown the same way as the live stack.
//
void stack_dump_words(const uint32_t* words, uint32_t count, uint32_t esp, uint32_t ebp);
