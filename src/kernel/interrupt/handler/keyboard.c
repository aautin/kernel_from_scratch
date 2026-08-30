#include "io.h"

#define KEY_RELEASED_MASK 0x80

enum keyboard_special_scancode
{
	KEY_CAPS      = 0x3A, // Toggle caps lock state
	KEY_TAB       = 0x0F, // Switch screen
	KEY_CTRL      = 0x1D, // Switch color scheme
	KEY_BACKSPACE = 0x0E, // Delete last character
	KEY_SPACE     = 0x39, // Put a space character
};

static const uint8_t scancode_map[128] =
{
	//
	// 10 elements per row
	//
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8',
	'9', '0', '-', '=', 0, 0, 'q', 'w', 'e', 'r',
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
	'm', '.', '/', 0, '*', 0, 0,
};

static uint8_t is_lock = false;

void keyboard_interrupt_handler(uint32_t scancode)
{
	if (scancode & KEY_RELEASED_MASK)
	{
		return;
	}

	//
	// The scancode is out of bounds
	//
	if (scancode >= sizeof(scancode_map))
	{
		return;
	}

	//
	// The character is printable
	//
	if (scancode_map[scancode])
	{
		uint8_t c = scancode_map[scancode];
		if (is_lock && scancode_map[scancode] >= 'a'
					&& scancode_map[scancode] <= 'z')
		{
			c -= 32;
		}
		vga_putc(c);
		return;
	}

	//
	// Special keys handling
	//
	switch (scancode)
	{
		case KEY_SPACE:
			vga_putc(' ');
			break;
		case KEY_CAPS:
			is_lock ^= 1;
			break;
		case KEY_BACKSPACE:
			vga_remove_last_character();
			break;
		case KEY_TAB:
			vga_switch_next_screen();
			break;
		case KEY_CTRL:
			vga_switch_color();
			break;
		default:
			vga_putc('?');
	}
}
