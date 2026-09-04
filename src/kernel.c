#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tty.h"
#include "keyboard.h"
#include "gdt.h"
#include "shell.h"

void kernel_main(void)
{
    //Initialize the kernel
    register_gdt();
    terminal_initialize();
    shell_run();
}