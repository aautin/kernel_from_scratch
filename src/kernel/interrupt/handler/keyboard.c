#include <stdbool.h>

#include "terminal.h"
#include "char.h"
#include "shell.h"
#include "port.h"
#include "keyboard.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEY_RELEASED_MASK 0x80
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
	// Released digit scancodes are 0x82 ('1') to 0x8B ('0'), so subtracting
	// this offset gives the screens 0 to 8 for '1' to '9', and 9 for '0'.
	//
	KEY_SCREEN_SWITCH_OFFSET = 0x82,
};

static const char scancode_map[128] =
{
	//
	// 10 elements per row
	//
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8',
	'9', '0', '-', '=', 0, 0, 'q', 'w', 'e', 'r',
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0, 
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
	'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static bool is_lock = false;
static bool is_ctrl = false;

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
	if (scancode == KEY_CTRL_PRESSED)
	{
		is_ctrl = true;
		return;
	}

	if (scancode & KEY_RELEASED_MASK)
	{
		if (scancode == KEY_CTRL_RELEASED)
			is_ctrl = false;
		else if (is_ctrl)
		{
			//
			// Any released key that is not a digit gives an index above 9,
			// which terminal_switch_screen() ignores.
			//
			terminal_switch_screen((uint8_t) (scancode - KEY_SCREEN_SWITCH_OFFSET));
		}
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

		if (is_ctrl)
		{
			//
			// Ctrl + digit switches screen, so don't type the character
			//
			return;
		}

		uint8_t c = scancode_map[scancode];
		if (is_lock)
		{
			c = toupper(c);
		}

		terminal_putc_input(c);
		return;
	}
	
	if (is_ctrl && (scancode == KEY_ENTER || scancode == KEY_BACKSPACE))
	{
		//
		// Ctrl is only a modifier, so it blocks Enter and Backspace too
		//
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
