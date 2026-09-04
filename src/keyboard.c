#include <stdbool.h>
#include <stdint.h>
#include "io.h"
#include "tty.h"
#include "keyboard.h"
#include "printk.h"

#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define STATUS_OUTPUT_FULL 0x01 // Bit 0: Output buffer full (data ready to read)


static const char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,
    ' ',
    0, 0, 0, 0, 0, 0,
};

static const char scancode_shift_to_ascii[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*',
    0,
    0,
    0, 0, 0, 0, 0, 0,
};


/**
 * Polls the PS/2 controller until a scancode is available, then reads it.
 */

uint8_t ps2_read_scancode(void)
{
    while ((inb(PS2_STATUS_PORT) & STATUS_OUTPUT_FULL) == 0)
        continue; //wait until the output buffer is full
    return inb(PS2_DATA_PORT);
}



char keyboard_getchar(void)
{
    char ascii;
    static bool shift_pressed = false;

    while (true)
    {
        char ascii;
        uint8_t scancode = ps2_read_scancode();
    
        if ((scancode & 0x80) == 0) // key pressed 
        {
            if (scancode == 0x2A || scancode == 0x36) // Shift key pressed
            {
                shift_pressed = true;
            }
            else
            {
                ascii = shift_pressed ? scancode_shift_to_ascii[scancode] : scancode_to_ascii[scancode];
                if (ascii)
                {
                    return ascii;
                }
            }
        }
        else // key released
        {
            uint8_t released_key = scancode & 0x7F;
            if (released_key == 0x2A || released_key == 0x36) // Shift key released
            {
                shift_pressed = false;
            }

        }
    }

}