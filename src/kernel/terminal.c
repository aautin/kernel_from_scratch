#include <stdint.h>

#include "vga.h"
#include "terminal.h"

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

struct state_s
{
	uint8_t terminal_index;

	uint16_t cursor_x;
	uint16_t cursor_y;
	uint16_t scroll_y;
	
	uint8_t color_index;
	uint8_t colors[TERMINAL_COLOR_COUNT];
};
typedef struct state_s state_t;

static screen_cell_t terminals[TERMINAL_SCREEN_COUNT][TERMINAL_HEIGHT][TERMINAL_WIDTH]; 
static state_t       state =
{
	.cursor_x = 0,
	.cursor_y = 0,
	.scroll_y = 0,
	.color_index = 0,
	.colors = { VGA_WHITE, VGA_BLUE, VGA_RED }
};

static inline bool can_scroll_up()
{
	return state.scroll_y > 0;
}

static inline bool can_scroll_down()
{
	return state.scroll_y < TERMINAL_EXTRA_LINES;
}

static uint8_t cursor_y_to_vga()
{
	return state.cursor_y - state.scroll_y;
}

static void put_terminal()
{
	for (uint32_t x = 0; x < TERMINAL_WIDTH; x++)
	{
		for (uint32_t y = 0; y < TERMINAL_HEIGHT; y++)
		{
			const screen_cell_t cell = terminals[state.terminal_index][y + state.scroll_y][x];
			vga_set_cell(cell.character, cell.fg_color, cell.bg_color, x, y);
		}
	}
}

static void scroll_down()
{
	state.scroll_y++;
	put_terminal();
	vga_put_cursor(state.cursor_x, cursor_y_to_vga());
}

static void scroll_up()
{
	state.scroll_y--;
	put_terminal();
	vga_put_cursor(state.cursor_x, cursor_y_to_vga());
}	

void terminal_move(enum terminal_cursor_direction direction)
{
	switch (direction)
	{
		case TERMINAL_CURSOR_UP:
			if (state.cursor_y > 0)
			{
				state.cursor_y--;
				if (state.cursor_y < state.scroll_y && can_scroll_up())
				{
					scroll_up();
				}
			}
			break;
		case TERMINAL_CURSOR_DOWN:
			if (state.cursor_y < TERMINAL_HEIGHT - 1)
			{
				state.cursor_y++;
				if (state.cursor_y - state.scroll_y >= VGA_HEIGHT && can_scroll_down())
				{
					scroll_down();
				}
			}
			break;
		case TERMINAL_CURSOR_LEFT:
			if (state.cursor_x > 0)
			{
				state.cursor_x--;
			}
			break;
		case TERMINAL_CURSOR_RIGHT:
			if (state.cursor_x < TERMINAL_WIDTH - 1)
			{
				state.cursor_x++;
			}
			break;
	}
		
	vga_put_cursor(state.cursor_x, cursor_y_to_vga());
}

void terminal_init()
{
	state.terminal_index = 0;

	state.cursor_x = 0;
	state.cursor_y = 0;

	state.scroll_y = 0;

	state.color_index = 0;

	state.colors[0] = VGA_WHITE;
	state.colors[1] = VGA_BLUE;
	state.colors[2] = VGA_RED;

	for (uint32_t i = 0; i < TERMINAL_SCREEN_COUNT; i++)
	{
		for (uint32_t x = 0; x < TERMINAL_WIDTH; x++)
		{
			for (uint32_t y = 0; y < TERMINAL_HEIGHT; y++)
			{
				screen_cell_t* cell = &terminals[i][y][x];
				cell->fg_color = state.colors[state.color_index];
				cell->bg_color = VGA_BLACK;
				cell->character = ' ';
			}
		}
	}

	put_terminal();
	vga_put_cursor(state.cursor_x, cursor_y_to_vga());
}

void terminal_switch_color_scheme()
{
	state.color_index = (state.color_index + 1) % TERMINAL_COLOR_COUNT;
}

void terminal_switch_screen()
{
	state.terminal_index = (state.terminal_index + 1) % TERMINAL_SCREEN_COUNT;

	state.scroll_y = 0;
	state.cursor_x = 0;
	state.cursor_y = 0;

	put_terminal();
	vga_put_cursor(state.cursor_x, cursor_y_to_vga());
}

void terminal_putc(char c)
{
	if (c == '\n')
	{
		if (state.cursor_y + 1 >= TERMINAL_HEIGHT)
		{
			//
			// The cursor is on the last line, newline execution is impossible
			//
			return;
		}

		if (state.cursor_y - state.scroll_y + 1 >= VGA_HEIGHT)
		{
			//
			// The cursor is on the last visible line, scrolling down
			//
			scroll_down();
		}

		for (uint32_t x = state.cursor_x; x < TERMINAL_WIDTH; x++)
		{
			screen_cell_t* cell = &terminals[state.terminal_index][state.cursor_y][x];
			cell->fg_color = state.colors[state.color_index];
			cell->bg_color = VGA_BLACK;
			cell->character = ' ';
			vga_set_cell(' ', cell->fg_color, cell->bg_color, x, state.cursor_y - state.scroll_y);
		}
		
		state.cursor_x = 0;
		state.cursor_y++;
		vga_put_cursor(state.cursor_x, cursor_y_to_vga());

		return;
	}

	const uint8_t fg_color = state.colors[state.color_index];
	const uint8_t bg_color = VGA_BLACK;

	screen_cell_t* cell = &terminals[state.terminal_index][state.cursor_y][state.cursor_x];
	cell->fg_color = fg_color;
	cell->bg_color = bg_color;
	cell->character = c;

	vga_set_cell(c, fg_color, bg_color, state.cursor_x, state.cursor_y - state.scroll_y);

	if (state.cursor_x == TERMINAL_WIDTH - 1 && state.cursor_y == TERMINAL_HEIGHT - 1)
	{
		//
		// The cursor is on the last cell of the screen, the cursor cannot be moved forward
		//
		return;
	}
	
	state.cursor_x++;
	if (state.cursor_x >= TERMINAL_WIDTH)
	{
		state.cursor_x = 0;
		state.cursor_y++;
		
		if (state.cursor_y - state.scroll_y >= VGA_HEIGHT)
		{
			//
			// The cursor is not visible, scrolling down
			//
			scroll_down();
		}
	}
	vga_put_cursor(state.cursor_x, cursor_y_to_vga());
}

void terminal_putnbr(int32_t number)
{
	if (number == 0)
	{
		terminal_putc('0');
		return;
	}

	if (number < 0)
	{
		terminal_putc('-');
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
		terminal_putc((char) ('0' + reversed_number % 10));
		reversed_number /= 10;
	}
}

void terminal_puts(const char* message)
{
	for (uint32_t index = 0; message[index]; index++)
	{
		terminal_putc(message[index]);
	}
}

void terminal_del(uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		if (state.cursor_x == 0 && state.cursor_y == 0)
		{
			//
			// The cursor is on the first cell of the screen, deletion is impossible
			//
			return;
		}

		if (state.cursor_x == 0)
		{
			state.cursor_x = TERMINAL_WIDTH - 1;
			state.cursor_y--;
			
			if (state.cursor_y < state.scroll_y)
			{
				//
				// The cursor is not visible, scrolling up
				//
				state.scroll_y--;
				put_terminal();
			}
		}
		else
		{
			state.cursor_x--;
		}

		screen_cell_t* cell = &terminals[state.terminal_index][state.cursor_y][state.cursor_x];
		cell->fg_color = state.colors[state.color_index];
		cell->bg_color = VGA_BLACK;
		cell->character = ' ';

		vga_set_cell(' ', cell->fg_color, cell->bg_color, state.cursor_x, state.cursor_y - state.scroll_y);
		vga_put_cursor(state.cursor_x, cursor_y_to_vga());
	}
}
