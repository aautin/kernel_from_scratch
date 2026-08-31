#include <stdbool.h>
#include <stdint.h>
#include "io.h"
#include "tty.h"
#include "keyboard.h"

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


/**
 * Polls the PS/2 controller until a scancode is available, then reads it.
 */

uint8_t ps2_read_scancode(void)
{
    while ((inb(PS2_STATUS_PORT) & STATUS_OUTPUT_FULL) == 0)
        continue; //wait until the output buffer is full
    return inb(PS2_DATA_PORT);
}



void keyboard_event_loop(void)
{
    while (true)
    {
        uint8_t scancode = ps2_read_scancode();
        
        if ((scancode & 0x80) == 0) // key pressed 
        {
            char ascii = scancode_to_ascii[scancode];
            if (ascii)
            {
                terminal_putchar(ascii);
            }
        }
        else // key released
        {
            // Handle key release
        }

    }
}