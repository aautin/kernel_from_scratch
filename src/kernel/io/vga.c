#include <stdint.h>

#include "io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY_ADDRESS 0xB8000
#define VGA_SCREEN_COUNT 4

#define VGA_CURSOR_PORT_COMMAND 0x3D4
#define VGA_CURSOR_PORT_DATA    0x3D5
#define VGA_CURSOR_PORT_HIGH    0x0F
#define VGA_CURSOR_PORT_LOW     0x0E

static uint32_t cursor         = 0;
static uint8_t  current_screen = 0;

static uint16_t buffer[VGA_SCREEN_COUNT][VGA_WIDTH * VGA_HEIGHT] = { {' ' | 0x0F00} };

#define COLOR_SCHEME_COUNT (sizeof(color_schemes) / sizeof(color_schemes[0]))

static const uint8_t color_schemes[][2] =
{
	{ 0x0F, 0x00 }, // White  on black
	{ 0x0A, 0x00 }, // Green  on black
	{ 0x0C, 0x00 }, // Red    on black
	{ 0x0E, 0x00 }, // Yellow on black
};
static uint8_t color_index = 0;

static uint8_t get_color()
{
	return color_schemes[color_index][0] | color_schemes[color_index][1] << 4;
}

static void move_cursor(uint32_t position)
{
	cursor = position;
	uint16_t cursor_position = (uint16_t) position;

	outb(VGA_CURSOR_PORT_COMMAND, VGA_CURSOR_PORT_HIGH);
	outb(VGA_CURSOR_PORT_DATA, (uint8_t) (cursor_position & 0xFF));
	outb(VGA_CURSOR_PORT_COMMAND, VGA_CURSOR_PORT_LOW);
	outb(VGA_CURSOR_PORT_DATA, (uint8_t) ((cursor_position >> 8) & 0xFF));
}

void vga_remove_last_character()
{
	if (cursor > 0)
	{
		cursor--;
		buffer[current_screen][cursor] = ' ' | get_color() << 8;

		volatile uint16_t* video_memory = (volatile uint16_t*) VGA_MEMORY_ADDRESS;

		video_memory[cursor] = ' ' | get_color() << 8;
		move_cursor(cursor);
	}
}

void vga_putc(uint16_t c)
{
	volatile uint16_t* video_memory = (volatile uint16_t*) VGA_MEMORY_ADDRESS;
	uint8_t color = ((c & 0xFF00) >> 8) == 0 ? get_color() : (c & 0xFF00) >> 8;

	if (cursor >= 80 * 25)
	{
		move_cursor(0);
	}

	if (c == '\n')
	{
		uint32_t row = cursor / VGA_WIDTH;
		uint32_t row_start = row * VGA_WIDTH;
		uint32_t row_end = row_start + VGA_WIDTH;

		for (uint16_t i = cursor; i < row_end; i++)
		{
			buffer[current_screen][i] = ' ' | (color << 8);
			video_memory[i] = ' ' | (color << 8);
		}
		move_cursor(cursor + (80 - (cursor % 80)));
		return;
	}

	buffer[current_screen][cursor] = (uint16_t) c | color << 8;
	video_memory[cursor] = (uint16_t) c | color << 8;
	move_cursor(cursor + 1);
}

void vga_putnbr(int32_t number)
{
	if (number == 0)
	{
		vga_putc('0');
		return;
	}

	if (number < 0)
	{
		vga_putc('-');
		number = -number;
	}

	int32_t reversed_number = 0;
	int32_t digit_count = 0;

	while (number > 0)
	{
		reversed_number = reversed_number * 10 + number % 10;
		number /= 10;
		digit_count++;
	}

	for (int32_t i = 0; i < digit_count; i++)
	{
		vga_putc((char) ('0' + reversed_number % 10));
		reversed_number /= 10;
	}
}

void vga_puts(const char* message)
{
	for (uint32_t index = 0; message[index]; index++)
	{
		vga_putc(message[index]);
	}
}

void vga_put_screen()
{
	for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
	{
		vga_putc(buffer[current_screen][i]);
	}
}

void vga_switch_next_screen()
{
	current_screen = (current_screen + 1) % VGA_SCREEN_COUNT;
	move_cursor(0);
	vga_put_screen();
	move_cursor(0);
}

void vga_switch_color()
{
	color_index = (color_index + 1) % COLOR_SCHEME_COUNT;
}
