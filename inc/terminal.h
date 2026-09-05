#pragma once

#include <stdint.h>

enum terminal_cursor_direction
{
	TERMINAL_CURSOR_UP,
	TERMINAL_CURSOR_DOWN,
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
// Change the terminal color scheme for the next input/output.
//
void terminal_switch_color_scheme();

//
// Switch between the first and second terminal.
//
void terminal_switch_screen();

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
// terminal_get_input() returns true if there is an input, false otherwise.
//
void terminal_putc_input(char input);
bool terminal_get_input(char* buffer, uint32_t buffer_size);
void terminal_del_input(uint32_t count);

//
// A up or down move allows the manual scrolling of the terminal.
//
void terminal_move(enum terminal_cursor_direction direction);
