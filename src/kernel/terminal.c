#include <stdint.h>

#include "vga.h"
#include "terminal.h"
#include "string.h"
#include "mem.h"

#define TERMINAL_PROMPT "42> "

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

struct screen
{
	uint16_t cursor_x;
	uint16_t cursor_y;
	uint16_t scroll_y;

	//
	// Equal to -1 when there is no current input field (e.g. when printing output).
	//
	int8_t input_size;

	screen_cell_t data[TERMINAL_HEIGHT][TERMINAL_WIDTH]; 
};
typedef struct screen screen_t;

struct state_s
{
	uint8_t terminal_index;
	uint8_t color_index;
	uint8_t colors[TERMINAL_COLOR_COUNT];
};
typedef struct state_s state_t;

static screen_t screens[TERMINAL_SCREEN_COUNT];
static state_t  state;

static screen_t* current_screen()
{
	return &screens[state.terminal_index];
}

static void put_screen()
{
	for (uint32_t x = 0; x < TERMINAL_WIDTH; x++)
	{
		for (uint32_t y = 0; y < TERMINAL_HEIGHT; y++)
		{
			const screen_cell_t* cell = &current_screen()->data[y][x];
			vga_set_cell(cell->character, cell->fg_color, cell->bg_color,
				x, y - screens[state.terminal_index].scroll_y);
		}
	}
}

static bool is_cursor_above_visible()
{
	return current_screen()->cursor_y < current_screen()->scroll_y;
}

static bool is_cursor_below_visible()
{
	return current_screen()->cursor_y >= current_screen()->scroll_y + VGA_HEIGHT;
}

static inline bool can_scroll_up()
{
	return current_screen()->scroll_y > 0;
}

static inline bool can_scroll_down()
{
	return (current_screen()->scroll_y + VGA_HEIGHT < TERMINAL_HEIGHT) && is_cursor_below_visible();
}

static uint8_t y_to_vga(uint8_t y)
{
	return y - current_screen()->scroll_y;
}

static void scroll_down()
{
	screen_t* screen = current_screen();

	screen->scroll_y++;
	put_screen();
	vga_put_cursor(screen->cursor_x, y_to_vga(screen->cursor_y));
}

static void scroll_up()
{
	screen_t* screen = current_screen();

	screen->scroll_y--;
	put_screen();
	vga_put_cursor(screen->cursor_x, y_to_vga(screen->cursor_y));
}

static void move_screen_up()
{
	screen_t* screen = current_screen();

	memmove(&screen->data[0][0], &screen->data[1][0],
		sizeof(screen_cell_t) * TERMINAL_WIDTH * (TERMINAL_HEIGHT - 1));
	for (uint32_t x = 0; x < TERMINAL_WIDTH; x++)
	{
		screen_cell_t* cell = &screen->data[TERMINAL_HEIGHT - 1][x];
		cell->fg_color = state.colors[state.color_index];
		cell->bg_color = VGA_BLACK;
		cell->character = ' ';
	}
	screen->cursor_y--;
}

static void align_scroll_to_cursor()
{
	while (is_cursor_above_visible())
	{
		scroll_up();
	}
	while(is_cursor_below_visible())
	{
		scroll_down();
	}
}

static void putc(char c)
{
	screen_t* screen = current_screen();
	
	if (c == '\n')
	{
		if (screen->cursor_y + 1 >= TERMINAL_HEIGHT)
		{
			//
			// The cursor is on the last line
			// We move up the whole screen to make space for the new line
			//
			move_screen_up();
		}
		
		for (uint32_t x = screen->cursor_x; x < TERMINAL_WIDTH; x++)
		{
			screen_cell_t* cell = &screen->data[screen->cursor_y][x];
			cell->fg_color = state.colors[state.color_index];
			cell->bg_color = VGA_BLACK;
			cell->character = ' ';
			vga_set_cell(' ', cell->fg_color, cell->bg_color, x, y_to_vga(screen->cursor_y));
		}
		
		
		screen->cursor_x = 0;
		screen->cursor_y++;
		align_scroll_to_cursor();
		vga_put_cursor(screen->cursor_x, y_to_vga(screen->cursor_y));
		
		return;
	}
	
	screen_cell_t* cell = &screen->data[screen->cursor_y][screen->cursor_x];
	cell->fg_color = state.colors[state.color_index];
	cell->bg_color = VGA_BLACK;
	cell->character = c;
	
	vga_set_cell(c, cell->fg_color, cell->bg_color, screen->cursor_x, y_to_vga(screen->cursor_y));
	
	screen->cursor_x++;
	if (screen->cursor_x >= TERMINAL_WIDTH)
	{
		screen->cursor_x = 0;
		screen->cursor_y++;
		
		if (screen->cursor_y >= TERMINAL_HEIGHT)
		{
			//
			// The cursor is on the last line
			// We move up the whole screen to make space for the new line
			//
			move_screen_up();
		}	
	}
	
	vga_put_cursor(screen->cursor_x, y_to_vga(screen->cursor_y));
	align_scroll_to_cursor();
	put_screen();
}

static void put_prompt()
{
	screen_t* screen = current_screen();
	for (uint32_t i = 0; TERMINAL_PROMPT[i]; i++)
	{
		screen_cell_t* cell = &screen->data[screen->cursor_y][screen->cursor_x];
		cell->fg_color  = state.colors[state.color_index];
		cell->bg_color  = VGA_BLACK;
		cell->character = TERMINAL_PROMPT[i];

		putc(TERMINAL_PROMPT[i]);
	}
}

void terminal_init()
{
	state.terminal_index = 0;
	state.color_index    = 0;
	
	state.colors[0] = VGA_WHITE;
	state.colors[1] = VGA_BLUE;
	state.colors[2] = VGA_RED;
	
	for (uint32_t i = 0; i < TERMINAL_SCREEN_COUNT; i++)
	{
		state.terminal_index = i;

		current_screen()->cursor_x = 0;
		current_screen()->cursor_y = 0;
		current_screen()->scroll_y = 0;

		terminal_clear();
		put_prompt();
	}

	put_screen();
	vga_put_cursor(current_screen()->cursor_x, y_to_vga(current_screen()->cursor_y));
	vga_set_cursor_visibility(true);
}

void terminal_clear()
{
	screen_t* screen = current_screen();

	for (uint32_t x = 0; x < TERMINAL_WIDTH; x++)
	{
		for (uint32_t y = 0; y < TERMINAL_HEIGHT; y++)
		{
			screen_cell_t* cell = &screen->data[y][x];
			
			cell->fg_color  = state.colors[state.color_index];
			cell->bg_color  = VGA_BLACK;
			cell->character = ' ';
		}
	}
	screen->cursor_x = 0;
	screen->cursor_y = 0;
	screen->scroll_y = 0;
	screen->input_size = 0;
}

void terminal_switch_color_scheme()
{
	state.color_index = (state.color_index + 1) % TERMINAL_COLOR_COUNT;
}

void terminal_switch_screen()
{
	state.terminal_index = (state.terminal_index + 1) % TERMINAL_SCREEN_COUNT;

	put_screen();
	vga_put_cursor(current_screen()->cursor_x, y_to_vga(current_screen()->cursor_y));
}

void terminal_begin_output()
{
	screen_t* screen = current_screen();
	putc('\n');
	screen->input_size = -1;

	vga_set_cursor_visibility(false);
}

void terminal_end_output()
{
	screen_t* screen = current_screen();
	if (screen->cursor_x != 0)
	{
		putc('\n');
	}
	put_prompt();
	screen->input_size = 0;
	vga_set_cursor_visibility(true);
}

void terminal_putc_output(char c)
{
	putc(c);
}

void terminal_putnbr_output(int32_t number)
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

void terminal_puts_output(const char* message)
{
	for (uint32_t index = 0; message[index]; index++)
	{
		putc(message[index]);
	}
}

void terminal_putc_input(char input)
{
	putc(input);
	current_screen()->input_size++;
}

bool terminal_get_input(char* buffer, uint32_t buffer_size)
{
    screen_t* screen = current_screen();
    if (screen->input_size < 0)
    {
        //
        // The terminal is not in input mode
        //
        return false;
    }

    if ((int32_t) buffer_size < screen->input_size + 1)
    {
        //
        // The buffer is too small to hold the input
        //
        return false;
    }

    for (uint32_t i = 0; i < (uint32_t) screen->input_size; i++)
    {
        int32_t position = (screen->cursor_x - screen->input_size) + i;
        uint32_t x = position % TERMINAL_WIDTH;
        uint32_t y = screen->cursor_y + (position / TERMINAL_WIDTH);

        const screen_cell_t* cell = &screen->data[y][x];
        buffer[i] = cell->character;
    }
    buffer[screen->input_size] = '\0';

    return true;
}

void terminal_del_input(uint32_t count)
{
	screen_t* screen = current_screen();
	if (screen->input_size < 0)
	{
		//
		// The terminal is not in input mode
		//
		return;
	}

	if (count > (uint32_t) screen->input_size)
	{
		//
		// The count exceeds the current input size, so we delete all input
		//
		count = (uint32_t) screen->input_size;
	}

	for (uint32_t i = 0; i < count; i++)
	{
		if (screen->cursor_x == 0 && screen->cursor_y == 0)
		{
			break;
		}

		if (screen->cursor_x == 0)
		{
			screen->cursor_y--;
			screen->cursor_x = TERMINAL_WIDTH - 1;
		}
		else
		{
			screen->cursor_x--;
		}

		uint32_t x = screen->cursor_x;
		uint32_t y = screen->cursor_y;

		screen_cell_t* cell = &screen->data[y][x];
		cell->fg_color = state.colors[state.color_index];
		cell->bg_color = VGA_BLACK;
		cell->character = ' ';

		vga_set_cell(' ', cell->fg_color, cell->bg_color, x, y_to_vga(y));
	}
	screen->input_size -= count;
	vga_put_cursor(screen->cursor_x, y_to_vga(screen->cursor_y));
}

void terminal_move(enum terminal_cursor_direction direction)
{
	screen_t* screen = current_screen();
	if (screen->input_size < 0)
	{
		//
		// The terminal is not in input mode
		//
		return;
	}

	switch (direction)
	{
	case TERMINAL_CURSOR_UP:
		if (can_scroll_up())
		{
			scroll_up();
		}
		break;
	case TERMINAL_CURSOR_DOWN:
		if (can_scroll_down())
		{
			scroll_down();
		}
		break;
	
	default:
		break;
	}
}