#include <stdbool.h>

#include "terminal.h"
#include "char.h"
#include "shell.h"

#define KEY_RELEASED_MASK 0x80

enum keyboard_special_scancode
{
	KEY_CAPS      = 0x3A, // Toggle caps lock state
	KEY_TAB       = 0x0F, // Switch screen
	KEY_CTRL      = 0x1D, // Switch color scheme
	KEY_BACKSPACE = 0x0E, // Delete last character
	KEY_SPACE     = 0x39, // Put a space character
	KEY_ENTER     = 0x1C, // Execute a command

	KEY_LEFT  = 0x4B, // Move cursor left
	KEY_RIGHT = 0x4D, // Move cursor right
	KEY_UP    = 0x48, // Move cursor up
	KEY_DOWN  = 0x50, // Move cursor down
};

static const char scancode_map[128] =
{
	//
	// 10 elements per row
	//
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8',
	'9', '0', '-', '=', 0, 0, 'q', 'w', 'e', 'r',
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0, 
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
	'm', '.', '/', 0, '*', 0, 0, 0, ' ', 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static bool is_lock = false;

static void execute()
{
	char buffer[256];
	if (terminal_last_word(buffer, sizeof(buffer)))
	{
		shell_execute(buffer);
	}
}

void keyboard_interrupt_handler(uint32_t scancode)
{
	if (scancode & KEY_RELEASED_MASK)
	{
		return;
	}

	if (scancode >= sizeof(scancode_map))
	{
		//
		// The scancode is out of bounds
		//
		return;
	}

	if (scancode_map[scancode])
	{
		//
		// The character is printable
		//

		uint8_t c = scancode_map[scancode];
		if (is_lock)
		{
			c = toupper(c);
		}

		terminal_putc(c);
		return;
	}

	switch (scancode)
	{
		//
		// Special keys handling
		//
		case KEY_ENTER:
			execute();
			break;
		case KEY_SPACE:
			terminal_putc(' ');
			break;
		case KEY_CAPS:
			is_lock ^= 1;
			break;
		case KEY_BACKSPACE:
			terminal_del(1);
			break;
		case KEY_TAB:
			terminal_switch_screen();
			break;
		case KEY_CTRL:
			terminal_switch_color_scheme();
			break;
		
		case KEY_LEFT:
			terminal_move(TERMINAL_CURSOR_LEFT);
			break;
		case KEY_RIGHT:
			terminal_move(TERMINAL_CURSOR_RIGHT);
			break;
		case KEY_UP:
			terminal_move(TERMINAL_CURSOR_UP);
			break;
		case KEY_DOWN:
			terminal_move(TERMINAL_CURSOR_DOWN);
			break;
		
		default:
			terminal_putc('?');
	}
}
