#pragma once

#include <stdint.h>

enum terminal_cursor_direction
{
	TERMINAL_CURSOR_UP,
	TERMINAL_CURSOR_DOWN,
	TERMINAL_CURSOR_LEFT,
	TERMINAL_CURSOR_RIGHT,
};

void terminal_init();
void terminal_clear();
void terminal_move(enum terminal_cursor_direction direction);

void terminal_switch_color_scheme();
void terminal_switch_screen();

void terminal_putc(char c);
void terminal_putnbr(int32_t number);
void terminal_puts(const char* message);
void terminal_del(uint32_t count);
bool terminal_last_word(char buffer[], uint32_t buffer_size);
