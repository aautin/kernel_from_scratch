#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tty.h"
#include "keyboard.h"

void kernel_main(void)
{
    //Initialize the kernel
    terminal_initialize();
    terminal_writestring("42\n");
    keyboard_event_loop();
}