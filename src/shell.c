#include "shell.h"
#include "keyboard.h"
#include "printk.h"
#include <stdbool.h>
#include <stdint.h>
#include "lib.h"
#include "io.h"

static void print_kernel_stack(void)
{
    uint32_t esp;
    uint32_t* stack_ptr;

    asm volatile("movl %%esp, %0" : "=r"(esp));
    stack_ptr = (uint32_t*)esp;

    printk("ESP = %p\n", (unsigned long)esp);
    for (int i = 0; i < 16; i++)
        printk("%p: %x\n", (unsigned long)(stack_ptr + i), stack_ptr[i]);
}

static void print_help(void)
{
    printk("Available commands:\n");
    printk("help - Show this help message\n");
    printk("clear - Clear the terminal screen\n");
    printk("print-kernel-stack - Print the current kernel stack\n");
    printk("reboot - Reboot the system\n");
}

static void reboot(void)
{
    unsigned char status = inb(0x64);
    while (status & 0x02)
    {
        status = inb(0x64);
    }
    outb(0xFE, 0x64);
}

void shell_execute(const char* command)
{
    if (command[0] == '\0')
    {
        return; // Ignore empty commands
    }
    else if (strcmp(command, "help") == 0)
    {
        print_help();
    }
    else if (strcmp(command, "clear") == 0)
    {
        terminal_initialize();
    }
    else if (strcmp(command, "print-kernel-stack") == 0)
    {
        printk("-------------\n");
        print_kernel_stack();
        printk("-------------\n");
    }
    else if (strcmp(command, "reboot") == 0)
    {
        reboot();
    }
    else
    {
        printk("Unknown command: ");
        printk("%s\n", command);
    }
}

void shell_run(void)
{
    char line[256];
    int i = 0;;

    printk("42> ");
    while (true)
    {
        char c = keyboard_getchar();
        if (c == '\n')
        {
            terminal_putchar(c);
            line[i] = '\0';
            shell_execute(line);
            i = 0;
            printk("42> ");
        }
        else if (c == '\b')
        {
            if (i > 0)
            {
                i--;
                terminal_putchar(c);
            }
        }
        else if (i < sizeof(line) - 1)
        {
            line[i++] = c;
            terminal_putchar(c);
        }
    }
}