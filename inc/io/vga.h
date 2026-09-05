#pragma once

#include <stdint.h>

enum vga_size
{
	VGA_WIDTH  = 80,
	VGA_HEIGHT = 25,
};

enum vga_color
{
	VGA_BLACK         = 0,
	VGA_BLUE          = 1,
	VGA_GREEN         = 2,
	VGA_CYAN          = 3,
	VGA_RED           = 4,
	VGA_MAGENTA       = 5,
	VGA_BROWN         = 6,
	VGA_LIGHT_GREY    = 7,
	VGA_DARK_GREY     = 8,
	VGA_LIGHT_BLUE    = 9,
	VGA_LIGHT_GREEN   = 10,
	VGA_LIGHT_CYAN    = 11,
	VGA_LIGHT_RED     = 12,
	VGA_LIGHT_MAGENTA = 13,
	VGA_LIGHT_BROWN   = 14,
	VGA_WHITE         = 15,
	VGA_COLOR_COUNT   = 16,
};

void vga_put_cursor(uint8_t x, uint8_t y);
void vga_set_cell(uint8_t c, uint8_t fg_color, uint8_t bg_color, uint8_t x, uint8_t y);
void vga_set_cursor_visibility(bool visible);
