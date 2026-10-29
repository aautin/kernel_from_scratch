#include "terminal.h"
#include <stdbool.h>

#include "printk.h"
#include "port.h"
#include "multiboot.h"
#include "string.h"
#include "stack.h"
#include "keyboard.h"
#include "cpu.h"

#define COMMAND_NAME_MAX_LENGTH        256
#define COMMAND_DESCRIPTION_MAX_LENGTH 256
#define COMMAND_COUNT                  10

#define PS2_CONTROLLER_STATUS_PORT     0x64
#define PS2_INPUT_BUFFER_FULL          0x02
#define PS2_REBOOT_COMMAND             0xFE

struct shell_command
{
	char name[COMMAND_NAME_MAX_LENGTH];
	char description[COMMAND_DESCRIPTION_MAX_LENGTH];

	void (*function)();
};
typedef struct shell_command shell_command_t;

struct shell
{
	multiboot_info_t* mbi;
	shell_command_t   commands[COMMAND_COUNT];
	shell_command_t*  last_command;
	bool              read_pending;
};
typedef struct shell shell_t;

static shell_t shell;

static void help()
{
	printk("Available commands:\n");
	for (int i = 0; i < COMMAND_COUNT; i++)
	{
		printk("%s: %s\n", shell.commands[i].name, shell.commands[i].description);
	}
}

static void clear()
{
	terminal_clear();
}

static void last()
{
	if (shell.last_command)
	{
		shell.last_command->function();
	}
	else
	{
		printk("No last command executed.\n");
	}
}

static void pmultiboot()
{
	print_multiboot_info(shell.mbi);
}

static void pstack(void)
{
	uint32_t* ebp;
	uint32_t* esp;

	asm volatile("mov %%ebp, %0" : "=r"(ebp));
	asm volatile("mov %%esp, %0" : "=r"(esp));

	stack_dump((uint32_t) esp, (uint32_t) ebp);
}

static void read()
{
	if (shell.read_pending)
		return;

	shell.read_pending = true;
}

static void halt()
{
	printk("System halted.\n");
	cpu_halt();
}

static void reboot()
{
	uint8_t status;
	do
	{
		status = inb(PS2_CONTROLLER_STATUS_PORT);
	}
	while (status & PS2_INPUT_BUFFER_FULL);

	outb(PS2_CONTROLLER_STATUS_PORT, PS2_REBOOT_COMMAND);
}

static void azerty()
{
	keyboard_set_layout(KEYBOARD_LAYOUT_AZERTY);
}

static void qwerty()
{
	keyboard_set_layout(KEYBOARD_LAYOUT_QWERTY);
}

static void add_shell_cmd(uint8_t index, const char* name,
	const char* desc, void (*command)())
{
	if (index < COMMAND_COUNT)
	{
		shell_command_t* cmd = &shell.commands[index];
		cmd->function = command;

		strncpy(cmd->name,        name, COMMAND_NAME_MAX_LENGTH - 1);
		strncpy(cmd->description, desc, COMMAND_DESCRIPTION_MAX_LENGTH - 1);
	}
}

void shell_init(multiboot_info_t* mbi)
{
	shell.mbi          = mbi;
	shell.last_command = 0;
	shell.read_pending = false;

	add_shell_cmd(0, "help",       "Display the commands list",             &help);
	add_shell_cmd(1, "clear",      "Clear the terminal screen",             &clear);
	add_shell_cmd(2, "last",       "Execute the last command",              &last);
	add_shell_cmd(3, "pmultiboot", "Print multiboot infos",                 &pmultiboot);
	add_shell_cmd(4, "pstack",     "Print the kernel stack",                &pstack);
	add_shell_cmd(5, "reboot",     "Reboot the system",                     &reboot);
	add_shell_cmd(6, "halt",       "Halt the system",                       &halt);
	add_shell_cmd(7, "read",       "Read a line and print it",              &read);
	add_shell_cmd(8, "azerty",     "Switch to the Azerty keyboard layout",  &azerty);
	add_shell_cmd(9, "qwerty",     "Switch to the Qwerty keyboard layout",  &qwerty);
}

bool shell_execute(const char* command)
{
	if (shell.read_pending)
	{
		terminal_begin_output();
		printk("read: \"%s\"", command);
		terminal_end_output(true);

		shell.read_pending = false;
		return true;
	}

	for (int i = 0; i < COMMAND_COUNT; i++)
	{
		if (strcmp(command, shell.commands[i].name) == 0)
		{
			terminal_begin_output();

			shell.commands[i].function();

			if (strcmp(command, "last") != 0)
			{
				shell.last_command = &shell.commands[i];
			}
			
			//
			// No prompt while the read command waits for its line
			//
			terminal_end_output(!shell.read_pending);
			return true;
		}
	}

	terminal_begin_output();
	if (command[0] != '\0')
		printk("%s: command not found\n", command);
	terminal_end_output(true);

	return false;
}

bool is_reading()
{
	return shell.read_pending;
}
