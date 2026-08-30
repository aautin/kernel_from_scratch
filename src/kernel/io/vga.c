#include <stdint.h>

#include "io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY_ADDRESS 0xB8000
#define VGA_SCREEN_COUNT 4
#define VGA_EXTRA_LINES VGA_HEIGHT

#define VGA_CURSOR_PORT_COMMAND 0x3D4
#define VGA_CURSOR_PORT_DATA    0x3D5
#define VGA_CURSOR_PORT_HIGH    0x0F
#define VGA_CURSOR_PORT_LOW     0x0E

#define COLOR_SCHEME_COUNT (sizeof(color_schemes) / sizeof(color_schemes[0]))
#define CELLS_PER_SCREEN (VGA_WIDTH * (VGA_HEIGHT + VGA_EXTRA_LINES))

static const uint8_t color_schemes[][2] =
{
	{ 0x0F, 0x00 }, // White  on black
	{ 0x0A, 0x00 }, // Green  on black
	{ 0x0C, 0x00 }, // Red    on black
	{ 0x0E, 0x00 }, // Yellow on black
};

static uint16_t screens[VGA_SCREEN_COUNT][CELLS_PER_SCREEN] = { {' ' | 0x0F00} };
static int32_t  cursor          = 0;
static uint8_t  current_screen  = 0;
static uint8_t  color_index     = 0;
static uint8_t  vertical_offset = 0;

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

static uint32_t screen_index(uint32_t index)
{
	return index + (vertical_offset * VGA_WIDTH);
}

static inline bool can_scroll_down()
{
	return vertical_offset < VGA_EXTRA_LINES;
}

static inline bool can_scroll_up()
{
	return vertical_offset > 0;
}

static void scroll_down()
{
	vertical_offset++;
	vga_put_screen();
	move_cursor(cursor - VGA_WIDTH);
}

static void scroll_up()
{
	vertical_offset--;
	vga_put_screen();
	move_cursor(cursor + VGA_WIDTH);
}

void vga_clear_screen(void)
{
    for (uint32_t s = 0; s < VGA_SCREEN_COUNT; s++)
    {
        for (uint32_t i = 0; i < CELLS_PER_SCREEN; i++)
            screens[s][i] = ' ' | 0x0F00;
    }
}

void vga_move_cursor(enum direction direction)
{
	int32_t offset = 0;
	switch (direction)
	{
		case VGA_CURSOR_LEFT:
			offset = -1;
			break;
		case VGA_CURSOR_RIGHT:
			offset = 1;
			break;
		case VGA_CURSOR_UP:
			offset = -VGA_WIDTH;
			break;
		case VGA_CURSOR_DOWN:
			offset = VGA_WIDTH;
			break;
	}

	if (offset + cursor >= VGA_WIDTH * VGA_HEIGHT)
	{
		if (can_scroll_down())
		{
			scroll_down();
		}
		else
		{
			return;
		}
	}

	if (offset + cursor < 0)
	{
		if (can_scroll_up())
		{
			scroll_up();
		}
		else
		{
			return;
		}
	}
	
	move_cursor(cursor + offset);
}

void vga_remove_last_character()
{
	if (cursor == 0 && can_scroll_up())
	{
		scroll_up();
	}

	if (cursor > 0)
	{
		screens[current_screen][screen_index(cursor) - 1] = ' ' | get_color() << 8;

		volatile uint16_t* vga = (volatile uint16_t*) VGA_MEMORY_ADDRESS;

		vga[cursor - 1] = ' ' | get_color() << 8;
		move_cursor(cursor - 1);
	}
}

void vga_putc(uint16_t c)
{
	volatile uint16_t* vga = (volatile uint16_t*) VGA_MEMORY_ADDRESS;
	uint8_t color = ((c & 0xFF00) >> 8) == 0 ? get_color() : (c & 0xFF00) >> 8;

	if (c == '\n')
	{
		if ((cursor / VGA_WIDTH) + 1 >= VGA_HEIGHT)
		{
			if (can_scroll_down())
			{
				scroll_down();
			}
			else
			{
				return;
			}
		}

		uint32_t row = cursor / VGA_WIDTH;
		uint32_t row_start = row * VGA_WIDTH;
		uint32_t row_end = row_start + VGA_WIDTH;

		for (uint16_t i = cursor; i < row_end; i++)
		{
			screens[current_screen][screen_index(i)] = ' ' | (color << 8);
			vga[i] = ' ' | (color << 8);
		}
		move_cursor(cursor + (VGA_WIDTH - (cursor % VGA_WIDTH)));
		return;
	}

	if (cursor + 1 >= VGA_WIDTH * VGA_HEIGHT)
	{
		if (can_scroll_down())
		{
			scroll_down();
		}
		else
		{
			return;
		}
	}

	screens[current_screen][screen_index(cursor)] = (uint16_t) c | color << 8;
	vga[cursor] = (uint16_t) c | color << 8;
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
	volatile uint16_t* vga = (volatile uint16_t*) VGA_MEMORY_ADDRESS;

    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        vga[i] = screens[current_screen][screen_index(i)];
    }
}

void vga_switch_next_screen()
{
	current_screen = (current_screen + 1) % VGA_SCREEN_COUNT;
	vertical_offset = 0;
	vga_put_screen();
	move_cursor(0);
}

void vga_switch_color()
{
	color_index = (color_index + 1) % COLOR_SCHEME_COUNT;
}
