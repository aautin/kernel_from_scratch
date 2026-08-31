#include <stdint.h>

#include "vga.h"

enum terminal
{
	TERMINAL_SCREEN_COUNT = 2,
	TERMINAL_EXTRA_LINES  = VGA_HEIGHT,
	TERMINAL_WIDTH        = VGA_WIDTH,
	TERMINAL_HEIGHT       = VGA_HEIGHT + TERMINAL_EXTRA_LINES,
	TERMINAL_COLOR_COUNT  = 3
};

struct screen_cell
{
	uint8_t fg_color;
	uint8_t bg_color;
	uint8_t character;
};
typedef struct screen_cell screen_cell_t;

struct terminal_s
{
	screen_cell_t screens[TERMINAL_SCREEN_COUNT][TERMINAL_HEIGHT][TERMINAL_WIDTH];
	uint8_t       screen_index;
	
	uint16_t cursor_x;
	uint16_t cursor_y;
	
	uint16_t scroll_y;

	uint8_t       color_index;
	const uint8_t colors[TERMINAL_COLOR_COUNT];
};
typedef struct terminal_s terminal_t;

static terminal_t terminal = {
	.screens      = { { 0 } },
	.screen_index = 0,

	.cursor_x     = 0,
	.cursor_y     = 0,

	.scroll_y     = 0,

	.color_index  = 0,
	.colors       = { VGA_WHITE, VGA_BLUE, VGA_RED },
};

static inline bool can_scroll_up()
{
	return terminal.scroll_y > 0;
}

static inline bool can_scroll_down()
{
	return terminal.scroll_y < TERMINAL_EXTRA_LINES;
}

static uint8_t vga_cursor_y()
{
	return terminal.cursor_y - terminal.scroll_y;
}

static void scroll_down()
{
	terminal.scroll_y++;
	put_screen();
	vga_put_cursor(terminal.cursor_x, vga_cursor_y());
}

static void scroll_up()
{
	terminal.scroll_y--;
	put_screen();
	vga_put_cursor(terminal.cursor_x, vga_cursor_y());
}

void switch_color_scheme()
{
	terminal.color_index = (terminal.color_index + 1) % TERMINAL_COLOR_COUNT;
}

void switch_screen()
{
	terminal.screen_index = (terminal.screen_index + 1) % TERMINAL_SCREEN_COUNT;
	terminal.scroll_y = 0;

	put_screen();
	vga_put_cursor(terminal.cursor_x, vga_cursor_y());
}

void put_screen()
{
	const screen_cell_t** screen = terminal.screens[terminal.screen_index];
	for (uint32_t x = 0; x < TERMINAL_WIDTH; x++)
	{
		for (uint32_t y = 0; y < TERMINAL_HEIGHT; y++)
		{
			const screen_cell_t cell = screen[x][y - terminal.scroll_y];
			vga_putc(cell.character, cell.fg_color, cell.bg_color, x, y);
		}
	}
}

void putc(char c)
{
	//
	// TODO
	//
}

void putnbr(int32_t number)
{
	if (number == 0)
	{
		putc('0');
		return;
	}

	if (number < 0)
	{
		putc('-');
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
		putc((char) ('0' + reversed_number % 10));
		reversed_number /= 10;
	}
}

void puts(const char* message)
{
	for (uint32_t index = 0; message[index]; index++)
	{
		putc(message[index]);
	}
}

void del(uint32_t count)
{
	//
	// TODO
	//
}
