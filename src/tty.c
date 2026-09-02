#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lib.h"
#include "tty.h"
#include "vga.h"
#include "io.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;


static void terminal_initialise_cursor(void)
{
	outb(0x0A,0x3D4);
	outb((inb(0x3D5) & 0xC0) | 0, 0x3D5);
	outb(0x0B,0x3D4);
	outb((inb(0x3D5) & 0xE0) | 15, 0x3D5);
}


void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	terminal_initialise_cursor();
	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

static void terminal_update_cursor(size_t col, size_t row)
{
	uint16_t pos = row * VGA_WIDTH + col;
	outb(0x0F, 0x3D4);
	outb((uint8_t)(pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t)((pos >> 8) & 0xFF), 0x3D5);
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t col, size_t row)
{
	const size_t index = row * VGA_WIDTH + col;
	terminal_buffer[index] = vga_entry(c, color);
}

static void terminal_scroll()
{
	for (size_t row = 1; row < VGA_HEIGHT; row++)
	{
		for (size_t col = 0; col < VGA_WIDTH; col++)
		{
			const size_t index = row * VGA_WIDTH + col;
			terminal_buffer[index - VGA_WIDTH] = terminal_buffer[index];
		}
	}
	for (size_t col = 0; col < VGA_WIDTH; col++)
	{
		const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + col;
		terminal_buffer[index] = vga_entry(' ', terminal_color);
	}
}

static void terminal_newline()
{
	terminal_column = 0;
	if (++terminal_row == VGA_HEIGHT)
	{
		terminal_scroll();
		terminal_row = VGA_HEIGHT - 1;
	}
}

void terminal_putchar(char c)
{
	unsigned char uc = c;
	if (uc == '\n')
	{
		terminal_newline();
	}
	else
	{
		terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH)
			terminal_newline();
	}
	terminal_update_cursor(terminal_column, terminal_row);
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}