#pragma once

#include <stdint.h>
#include <stdbool.h>

enum terminal_cursor_direction
{
	TERMINAL_CURSOR_UP,
	TERMINAL_CURSOR_DOWN,
	TERMINAL_CURSOR_LEFT,
	TERMINAL_CURSOR_RIGHT,
};

//
// Terminal state initialization.
// Put spaces into every cell of the terminal and put a first prompt.
//
void terminal_init();

//
// When terminal gets cleared, a new prompt is printed to be ready for input.
//
void terminal_clear();


//
// Show the screen at the given index (0 to 9), with its own color, buffer,
// cursor and scroll position. An out of range index is ignored.
//
void terminal_switch_screen(uint8_t index);

//
// Those functions handle the terminal output.
//
// The begin and the end functions are used to print a newline before the first
// output and a new prompt after the last output, and also to disable the cursor
// while the output print and at then end to move it to the next input
//
void terminal_begin_output();
void terminal_end_output();
void terminal_putc_output(char c);
void terminal_putnbr_output(int32_t number);
void terminal_puts_output(const char* message);

//
// Those functions handle the input.
// Cannot delete characters beyond the input.
//
// Characters are inserted at the cursor. terminal_del_input() deletes the
// characters before the cursor (backspace), terminal_del_input_forward() the
// characters after it (delete).
//
// terminal_get_input() returns true if there is an input, false otherwise.
//
void terminal_putc_input(char input);
bool terminal_get_input(char* buffer, uint32_t buffer_size);
void terminal_del_input(uint32_t count);
void terminal_del_input_forward(uint32_t count);

//
// A up or down move allows the manual scrolling of the terminal.
// A left or right move moves the cursor inside the input.
//
void terminal_move(enum terminal_cursor_direction direction);
