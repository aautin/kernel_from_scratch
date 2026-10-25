#include <stdbool.h>

#include "terminal.h"
#include "char.h"
#include "shell.h"
#include "port.h"
#include "keyboard.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEY_BUFFER_SIZE   128

enum keyboard_special_scancode
{
	KEY_CAPS      = 0x3A, // Toggle caps lock state
	KEY_BACKSPACE = 0x0E, // Delete last character
	KEY_ENTER     = 0x1C, // Execute a command
	KEY_DELETE    = 0x53, // Delete the character at the cursor

	
	KEY_LEFT  = 0x4B, // Move cursor left
	KEY_RIGHT = 0x4D, // Move cursor right
	KEY_UP    = 0x48, // Move cursor up
	KEY_DOWN  = 0x50, // Move cursor down
	
	//
	// Hold Ctrl and release a digit to switch screen
	//
	KEY_CTRL_PRESSED  = 0x1D,
	KEY_CTRL_RELEASED = 0x9D,

	//
	// Hold Shift to write in uppercase
	//
	KEY_LEFT_SHIFT_PRESS    = 0x2A,
	KEY_LEFT_SHIFT_RELEASE  = 0xAA,
};

#define SCANCODE_MAP_SIZE 128
static const char maps[2][SCANCODE_MAP_SIZE] =
{
	{
		//
		// Qwerty
		//

		[0x02] = '1', [0x03] = '2', [0x04] = '3',
		[0x05] = '4', [0x06] = '5', [0x07] = '6',
		[0x08] = '7', [0x09] = '8', [0x0A] = '9',
		[0x0B] = '0', [0x0C] = '-', [0x0D] = '=',

		[0x10] = 'q', [0x11] = 'w', [0x12] = 'e',
		[0x13] = 'r', [0x14] = 't', [0x15] = 'y',
		[0x16] = 'u', [0x17] = 'i', [0x18] = 'o',
		[0x19] = 'p', [0x1A] = '[', [0x1B] = ']',

		[0x1E] = 'a', [0x1F] = 's', [0x20] = 'd',
		[0x21] = 'f', [0x22] = 'g', [0x23] = 'h',
		[0x24] = 'j', [0x25] = 'k', [0x26] = 'l',
		[0x27] = ';', [0x28] = '\'',

		[0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c',
		[0x2F] = 'v', [0x30] = 'b', [0x31] = 'n',
		[0x32] = 'm', [0x33] = ',', [0x34] = '.',
		[0x35] = '/',

		[0x39] = ' ',
	},
	{
		//
		// Azerty
		//
		[0x02] = '&', [0x03] = 0x82, [0x04] = '"',
		[0x05] = '\'', [0x06] = '(', [0x07] = '-',
		[0x08] = 0x8A, [0x09] = '_', [0x0A] = 0x87,
		[0x0B] = 0x85, [0x0C] = ')', [0x0D] = '=',

		[0x10] = 'a', [0x11] = 'z', [0x12] = 'e',
		[0x13] = 'r', [0x14] = 't', [0x15] = 'y',
		[0x16] = 'u', [0x17] = 'i', [0x18] = 'o',
		[0x19] = 'p', [0x1A] = '^', [0x1B] = '$',

		[0x1E] = 'q', [0x1F] = 's', [0x20] = 'd',
		[0x21] = 'f', [0x22] = 'g', [0x23] = 'h',
		[0x24] = 'j', [0x25] = 'k', [0x26] = 'l',
		[0x27] = 'm',

		[0x2B] = '*',
		[0x2C] = 'w', [0x2D] = 'x', [0x2E] = 'c',
		[0x2F] = 'v', [0x30] = 'b', [0x31] = 'n',
		[0x32] = ',', [0x33] = ';', [0x34] = ':',
		[0x35] = '!',

		[0x39] = ' ',
	}
};

#define SCANCODE_UPPERMAP_SIZE 0x0E
static const char upper_maps[2][0x0E] = 
{
	{
		//
		// Qwerty
		//
		[0x02] = '!', [0x03] = '@', [0x04] = '#',
		[0x05] = '$', [0x06] = '%', [0x07] = '^',
		[0x08] = '&', [0x09] = '*', [0x0A] = '(',
		[0x0B] = ')', [0x0C] = '_', [0x0D] = '+',
	},
	{
		//
		// Azerty
		//
		[0x02] = '1', [0x03] = '2', [0x04] = '3',
		[0x05] = '4', [0x06] = '5', [0x07] = '6',
		[0x08] = '7', [0x09] = '8', [0x0A] = '9',
		[0x0B] = '0', [0x0C] = 0xB0, [0x0D] = '+',
	},
};

static bool is_lock  = false;
static bool is_ctrl  = false;
static bool is_shift = false;

static enum keyboard_layout current_layout = KEYBOARD_LAYOUT_QWERTY;

//
// Scancodes received by the interrupt handler, waiting to be handled by the
// main loop. The interrupt handler only writes the head and the main loop only
// writes the tail, so no lock is needed.
//
static volatile uint8_t  key_buffer[KEY_BUFFER_SIZE];
static volatile uint32_t key_buffer_head = 0;
static volatile uint32_t key_buffer_tail = 0;

static void execute()
{
	char buffer[256] = {0};
	if (terminal_get_input(buffer, sizeof(buffer)))
	{
		shell_execute(buffer);
	}
}

static void handle_scancode(uint32_t scancode)
{
	switch (scancode)
	{
		case KEY_LEFT_SHIFT_PRESS:
			is_shift = true;
			return;

		case KEY_LEFT_SHIFT_RELEASE:
			is_shift = false;
			return;

		case KEY_CTRL_PRESSED:
			is_ctrl = true;
			return;

		case KEY_CTRL_RELEASED:
			is_ctrl = false;
			return;

		default:
			break;
	}

	if (scancode >= SCANCODE_MAP_SIZE)
	{
		//
		// The scancode is out of bounds
		//
		return;
	}

	uint8_t c = maps[current_layout][scancode];
	if (is_lock ^ is_shift)
	{
		if (scancode < SCANCODE_UPPERMAP_SIZE)
		{
			c = upper_maps[current_layout][scancode];
		}
		else
		{
			c = toupper(c);
		}
	}

	if (is_ctrl)
	{
		//
		// Ctrl + digit switches screen in the keyboard row order: '1' is the
		// first screen (index 0) and '0' the last one (index 9)
		//
		if (!is_reading() && isdigit(c))
		{
			terminal_switch_screen((c - '1' + 10) % 10);
		}

		return;
	}

	if (c)
	{
		//
		// The character is printable
		//

		terminal_putc_input(c);
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
		case KEY_BACKSPACE:
			terminal_del_input(1);
			break;
		case KEY_DELETE:
			terminal_del_input_forward(1);
			break;
		case KEY_CAPS:
			is_lock ^= 1;
			break;
		case KEY_UP:
			terminal_move(TERMINAL_CURSOR_UP);
			break;
		case KEY_DOWN:
			terminal_move(TERMINAL_CURSOR_DOWN);
			break;
		case KEY_LEFT:
			terminal_move(TERMINAL_CURSOR_LEFT);
			break;
		case KEY_RIGHT:
			terminal_move(TERMINAL_CURSOR_RIGHT);
			break;
		default:
			//
			// Other keys, like Tab, do nothing
			//
			break;
	}
}

void keyboard_set_layout(enum keyboard_layout new_layout)
{
	current_layout = new_layout;
}

void keyboard_interrupt_handler(interrupt_frame_t* frame)
{
	(void) frame;
	keyboard_push_scancode(inb(KEYBOARD_DATA_PORT));
}

void keyboard_push_scancode(uint8_t scancode)
{
	uint32_t next_head = (key_buffer_head + 1) % KEY_BUFFER_SIZE;
	if (next_head == key_buffer_tail)
	{
		//
		// The buffer is full, the key is dropped
		//
		return;
	}

	key_buffer[key_buffer_head] = scancode;
	key_buffer_head = next_head;
}

bool keyboard_has_input()
{
	return key_buffer_head != key_buffer_tail;
}

void keyboard_handle_input()
{
	while (keyboard_has_input())
	{
		uint8_t scancode = key_buffer[key_buffer_tail];
		key_buffer_tail = (key_buffer_tail + 1) % KEY_BUFFER_SIZE;

		handle_scancode(scancode);
	}
}
