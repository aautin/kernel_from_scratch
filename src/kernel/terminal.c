#include <stdint.h>
#include <stdbool.h>

#include "vga.h"
#include "terminal.h"
#include "string.h"
#include "mem.h"

#define TERMINAL_PROMPT "$ "

enum terminal
{
	TERMINAL_SCREEN_COUNT = 10,
	TERMINAL_EXTRA_LINES  = 200 - VGA_HEIGHT, // 200 lines of scrollback per screen
	TERMINAL_WIDTH        = VGA_WIDTH,
	TERMINAL_HEIGHT       = VGA_HEIGHT + TERMINAL_EXTRA_LINES,
};

//
// Each screen has its own fixed color, stored as a VGA attribute byte:
// background in the high 4 bits, foreground in the low 4 bits.
//
static const uint8_t screen_colors[TERMINAL_SCREEN_COUNT] =
{
	0x07, 0x1F, 0x76, 0xF0, 0x30, 0x4E, 0x5F, 0x6F, 0x0A, 0x0C
};

//
// A cell is a 16-bit value with the VGA text layout: the character in the low
// byte, then the foreground and background colors.
//
struct screen_cell
{
	uint16_t character : 8;
	uint16_t fg_color  : 4;
	uint16_t bg_color  : 4;
} __attribute__((packed));
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

	//
	// Number of input characters before the cursor, which the left and right
	// arrows move inside the input.
	//
	int8_t input_cursor;

	uint8_t fg_color;
	uint8_t bg_color;

	//
	// A screen is cleared and gets its first prompt the first time it is shown.
	//
	bool initialized;

	screen_cell_t data[TERMINAL_HEIGHT][TERMINAL_WIDTH]; 
};
typedef struct screen screen_t;

struct state_s
{
	uint8_t terminal_index;
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
		//
		// Only the visible rows are drawn
		//
		for (uint32_t y = current_screen()->scroll_y; y < (uint32_t) current_screen()->scroll_y + VGA_HEIGHT; y++)
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
		cell->fg_color = screen->fg_color;
		cell->bg_color = screen->bg_color;
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

static uint32_t cursor_position()
{
	return current_screen()->cursor_y * TERMINAL_WIDTH + current_screen()->cursor_x;
}

static screen_cell_t* cell_at(uint32_t position)
{
	return &current_screen()->data[position / TERMINAL_WIDTH][position % TERMINAL_WIDTH];
}

static void move_cursor(int32_t offset)
{
	screen_t* screen = current_screen();
	uint32_t position = cursor_position() + offset;

	screen->cursor_x = position % TERMINAL_WIDTH;
	screen->cursor_y = position / TERMINAL_WIDTH;
	align_scroll_to_cursor();
	vga_put_cursor(screen->cursor_x, y_to_vga(screen->cursor_y));
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
			cell->fg_color = screen->fg_color;
			cell->bg_color = screen->bg_color;
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
	cell->fg_color = screen->fg_color;
	cell->bg_color = screen->bg_color;
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

//
// Copy the input characters from the cursor to the end of the input.
//
static uint32_t get_input_after_cursor(char* buffer)
{
	screen_t* screen = current_screen();
	uint32_t count = screen->input_size - screen->input_cursor;

	for (uint32_t i = 0; i < count; i++)
		buffer[i] = cell_at(cursor_position() + i)->character;
	return count;
}

//
// Print the characters from the cursor, then spaces over the erase_count
// characters left at the old end of the input, and move the cursor back.
//
static void put_input_after_cursor(const char* buffer, uint32_t count, uint32_t erase_count)
{
	for (uint32_t i = 0; i < count; i++)
		putc(buffer[i]);
	for (uint32_t i = 0; i < erase_count; i++)
		putc(' ');
	move_cursor(-(int32_t) (count + erase_count));
}

static void put_prompt()
{
	for (uint32_t i = 0; TERMINAL_PROMPT[i]; i++)
	{
		putc(TERMINAL_PROMPT[i]);
	}
}

void terminal_init()
{
	for (uint32_t i = 0; i < TERMINAL_SCREEN_COUNT; i++)
	{
		screens[i].fg_color    = screen_colors[i] & 0x0F;
		screens[i].bg_color    = screen_colors[i] >> 4;
		screens[i].initialized = false;
	}

	//
	// Only the first screen is set up now, the others are set up the first
	// time they are shown (see terminal_switch_screen()).
	//
	state.terminal_index = 0;
	terminal_clear();
	put_prompt();
	current_screen()->initialized = true;

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
			
			cell->fg_color  = screen->fg_color;
			cell->bg_color  = screen->bg_color;
			cell->character = ' ';
		}
	}
	screen->cursor_x = 0;
	screen->cursor_y = 0;
	screen->scroll_y = 0;
	screen->input_size = 0;
	screen->input_cursor = 0;
}

void terminal_switch_screen(uint8_t index)
{
	if (index >= TERMINAL_SCREEN_COUNT)
		return;
	state.terminal_index = index;

	if (!current_screen()->initialized)
	{
		terminal_clear();
		put_prompt();
		current_screen()->initialized = true;
	}

	//
	// Every cell of the buffer holds the screen color, so redrawing the buffer
	// also fills the whole VGA screen with it.
	//
	put_screen();
	vga_put_cursor(current_screen()->cursor_x, y_to_vga(current_screen()->cursor_y));
}

void terminal_begin_output()
{
	screen_t* screen = current_screen();
	if (screen->input_size > 0)
	{
		//
		// The new line goes after the whole input, not at the cursor
		//
		move_cursor(screen->input_size - screen->input_cursor);
	}
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
	screen->input_cursor = 0;
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

void terminal_begin_input()
{
	screen_t* screen = current_screen();
	screen->input_size   = 0;
	screen->input_cursor = 0;
	vga_set_cursor_visibility(true);
}

void terminal_end_input()
{
	screen_t* screen = current_screen();

	//
	// The new line goes after the whole input, not at the cursor
	//
	move_cursor(screen->input_size - screen->input_cursor);
	putc('\n');
	screen->input_size = -1;
	vga_set_cursor_visibility(false);
}

void terminal_putc_input(char input)
{
	screen_t* screen = current_screen();
	if (screen->input_size < 0 || screen->input_size == INT8_MAX)
	{
		//
		// The terminal is not in input mode, or the input is full
		//
		return;
	}

	//
	// The input after the cursor is moved one character to the right
	//
	char after_cursor[INT8_MAX];
	uint32_t count = get_input_after_cursor(after_cursor);

	putc(input);
	put_input_after_cursor(after_cursor, count, 0);

	screen->input_size++;
	screen->input_cursor++;
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

    uint32_t input_start = cursor_position() - screen->input_cursor;
    for (uint32_t i = 0; i < (uint32_t) screen->input_size; i++)
    {
        const screen_cell_t* cell = cell_at(input_start + i);
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

	if (count > (uint32_t) screen->input_cursor)
	{
		//
		// The count exceeds the input before the cursor, so we delete all of it
		//
		count = (uint32_t) screen->input_cursor;
	}

	char after_cursor[INT8_MAX];
	uint32_t after_count = get_input_after_cursor(after_cursor);

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
		cell->fg_color = screen->fg_color;
		cell->bg_color = screen->bg_color;
		cell->character = ' ';

		vga_set_cell(' ', cell->fg_color, cell->bg_color, x, y_to_vga(y));
	}

	//
	// The input after the cursor is moved count characters to the left
	//
	put_input_after_cursor(after_cursor, after_count, count);

	screen->input_size -= count;
	screen->input_cursor -= count;
	vga_put_cursor(screen->cursor_x, y_to_vga(screen->cursor_y));
}

void terminal_del_input_forward(uint32_t count)
{
	screen_t* screen = current_screen();
	if (screen->input_size < 0)
	{
		//
		// The terminal is not in input mode
		//
		return;
	}

	char after_cursor[INT8_MAX];
	uint32_t after_count = get_input_after_cursor(after_cursor);

	if (count > after_count)
	{
		//
		// The count exceeds the input after the cursor, so we delete all of it
		//
		count = after_count;
	}

	put_input_after_cursor(after_cursor + count, after_count - count, count);
	screen->input_size -= count;
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
	case TERMINAL_CURSOR_LEFT:
		if (screen->input_cursor > 0)
		{
			move_cursor(-1);
			screen->input_cursor--;
		}
		break;
	case TERMINAL_CURSOR_RIGHT:
		if (screen->input_cursor < screen->input_size)
		{
			move_cursor(1);
			screen->input_cursor++;
		}
		break;
	
	default:
		break;
	}
}