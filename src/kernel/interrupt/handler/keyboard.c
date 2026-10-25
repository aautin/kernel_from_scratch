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
	// Hold Shift to write in uppercase
	//
	KEY_LEFT_SHIFT_PRESS    = 0x2A,
	KEY_LEFT_SHIFT_RELEASE  = 0xAA,

	//
	// Released digit scancodes are 0x82 ('1') to 0x8B ('0'), so subtracting
	// this offset gives the screens 0 to 8 for '1' to '9', and 9 for '0'.
	//
	KEY_SCREEN_SWITCH_OFFSET = 0x82,
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

void keyboard_set_layout(enum keyboard_layout layout);

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

//
// Set by get_line() while it waits for a line: Enter then ends the line
// instead of executing it as a command.
//
static bool line_requested = false;
static bool line_done      = false;

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

	if (scancode == KEY_LEFT_SHIFT_PRESS)
	{
		is_shift = true;
		return;
	}
	
	if (scancode == KEY_LEFT_SHIFT_RELEASE)
	{
		is_shift = false;
		return;
	}

	if (scancode == KEY_CTRL_PRESSED)
	{
		is_ctrl = true;
		return;
	}

	if (scancode & KEY_RELEASED_MASK)
	{
		if (scancode == KEY_CTRL_RELEASED)
			is_ctrl = false;
		else if (is_ctrl && !line_requested)
		{
			//
			// Any released key that is not a digit gives an index above 9,
			// which terminal_switch_screen() ignores.
			// The screen doesn't change while get_line() waits for a line on it.
			//
			terminal_switch_screen((uint8_t) (scancode - KEY_SCREEN_SWITCH_OFFSET));
		}
		return;
	}

	if (scancode >= SCANCODE_MAP_SIZE)
	{
		//
		// The scancode is out of bounds
		//
		return;
	}

	if (maps[current_layout][scancode])
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

		uint8_t c = maps[current_layout][scancode];
		if (is_lock ^ is_shift)
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
			if (line_requested)
				line_done = true;
			else
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
	//
	// The keys typed after the Enter that ends a get_line() line are kept
	// for after get_line() returns
	//
	while (keyboard_has_input() && !line_done)
	{
		uint8_t scancode = key_buffer[key_buffer_tail];
		key_buffer_tail = (key_buffer_tail + 1) % KEY_BUFFER_SIZE;

		handle_scancode(scancode);
	}
}

uint32_t get_line(char* buffer, uint32_t size)
{
	if (size == 0)
		return 0;

	terminal_begin_input();
	line_requested = true;

	while (!line_done)
	{
		keyboard_handle_input();

		//
		// Same wait as in kernel_main(): interrupts are disabled while
		// checking the key buffer, so a key can't arrive between the check
		// and 'hlt'
		//
		__asm__ volatile ("cli");
		if (!line_done && !keyboard_has_input())
			__asm__ volatile ("sti; hlt");
		__asm__ volatile ("sti");
	}

	char line[256] = {0};
	terminal_get_input(line, sizeof(line));
	terminal_end_input();

	line_requested = false;
	line_done      = false;

	//
	// Like read(), the line ends with '\n'. It is cut if the buffer is too
	// small, and always ends with '\0'.
	//
	uint32_t length = 0;
	while (line[length] && length < size - 1)
	{
		buffer[length] = line[length];
		length++;
	}
	if (length < size - 1)
		buffer[length++] = '\n';
	buffer[length] = '\0';

	return length;
}
