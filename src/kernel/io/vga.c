#include <stdint.h>

#include "vga.h"

#define VGA_MEMORY_ADDRESS      0xB8000
#define VGA_CURSOR_PORT_COMMAND 0x3D4
#define VGA_CURSOR_PORT_DATA    0x3D5
#define VGA_CURSOR_PORT_HIGH    0x0F
#define VGA_CURSOR_PORT_LOW     0x0E

static uint8_t get_color(uint8_t foreground, uint8_t background)
{
	return (background << 4) | (foreground & 0x0F);
}

static uint16_t get_position(uint8_t row, uint8_t column)
{
	return (row * VGA_WIDTH) + column;
}

uint16_t vga_cell(uint8_t character, uint8_t fg_color, uint8_t bg_color)
{
	return (uint16_t) character | ((uint16_t) get_color(fg_color, bg_color) << 8);
}

void vga_set_cursor_visibility(bool visible)
{
	uint8_t cursor_start = inb(VGA_CURSOR_PORT_COMMAND + 0x0A);
	uint8_t cursor_end   = inb(VGA_CURSOR_PORT_COMMAND + 0x0B);

	//
	// TODO
	// Substitute the magic numbers with named constants.
	//
	if (visible)
	{
		cursor_start &= 0xC0; // Clear the cursor start bits
		cursor_end   &= 0xE0; // Clear the cursor end bits
	}
	else
	{
		cursor_start |= 0x20; // Set the cursor start bit to hide the cursor
	}

	outb(VGA_CURSOR_PORT_COMMAND + 0x0A, cursor_start);
	outb(VGA_CURSOR_PORT_COMMAND + 0x0B, cursor_end);
}

void vga_put_cursor(uint8_t x, uint8_t y)
{
	uint16_t position = get_position(y, x);

	outb(VGA_CURSOR_PORT_COMMAND, VGA_CURSOR_PORT_HIGH);
	outb(VGA_CURSOR_PORT_DATA, position & 0xFF);         // Send the low byte of the position
	outb(VGA_CURSOR_PORT_COMMAND, VGA_CURSOR_PORT_LOW);
	outb(VGA_CURSOR_PORT_DATA, position >> 8);           // Send the high byte of the position
}

void vga_put_cell(uint16_t cell, uint8_t x, uint8_t y)
{
	volatile uint16_t* vga      = (volatile uint16_t*) VGA_MEMORY_ADDRESS;
	const    uint16_t  position = get_position(y, x);

	vga[position] = cell;
}

void vga_clear(uint8_t fg_color, uint8_t bg_color)
{
	volatile uint16_t* vga = (volatile uint16_t*) VGA_MEMORY_ADDRESS;
	const    uint16_t  cell = vga_cell(' ', fg_color, bg_color);

	for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
	{
		vga[i] = cell;
	}
}

void vga_fill(const uint16_t* screen_buffer)
{
	volatile uint16_t* vga = (volatile uint16_t*) VGA_MEMORY_ADDRESS;

	for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
	{
		vga[i] = screen_buffer[i];
	}
}
