#include "terminal.h"
#include <stdbool.h>

#include "printk.h"
#include "port.h"
#include "multiboot.h"
#include "string.h"
#include "stack.h"
#include "keyboard.h"
#include "cpu.h"
#include "signal.h"
#include "timer.h"

#define COMMAND_NAME_MAX_LENGTH        256
#define COMMAND_DESCRIPTION_MAX_LENGTH 256
#define COMMAND_COUNT                  16

#define PS2_CONTROLLER_STATUS_PORT     0x64
#define PS2_INPUT_BUFFER_FULL          0x02
#define PS2_REBOOT_COMMAND             0xFE

#define SIGNAL_TEST_DELAY_MS           1000

enum shell_signal
{
	SHELL_SIGNAL_TEST         = 1, // Prints its delivery
	SHELL_SIGNAL_COUNT        = 2, // Silently counted, to fill the queue
	SHELL_SIGNAL_UNREGISTERED = 3, // Never registered
};

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
	bool              executing;
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

static const char* signal_status_name(signal_status_t status)
{
	switch (status)
	{
		case SIGNAL_SUCCESS:              return "success";
		case SIGNAL_ERROR_INVALID_NUMBER: return "invalid number";
		case SIGNAL_ERROR_NOT_REGISTERED: return "not registered";
		case SIGNAL_ERROR_QUEUE_FULL:     return "queue full";
	}
	return "unknown";
}

static const char* signal_name(uint8_t number)
{
	switch (number)
	{
		case SIGNAL_TRAP:               return " (SIGTRAP)";
		case SIGNAL_SEGMENTATION_FAULT: return " (SIGSEGV)";
	}
	return "";
}

static void on_test_signal(uint8_t number)
{
	//
	// A scheduled signal, or one queued by an exception handler, is delivered
	// by the main loop, outside of a command, so it has to open its own output
	//
	bool own_output = !shell.executing;
	if (own_output)
		terminal_begin_output();

	printk("[signal] %u%s delivered %s at tick %u\n", number, signal_name(number),
		own_output ? "later by the main loop" : "immediately", timer_ticks());

	if (own_output)
		terminal_end_output(!shell.read_pending);
}

static void on_count_signal(uint8_t number)
{
	(void) number;
}

static void raise_demo()
{
	printk("[raise] raise %u, delivered before signal_raise returns:\n",
		SHELL_SIGNAL_TEST);
	signal_status_t status = signal_raise(SHELL_SIGNAL_TEST);
	printk("[raise] signal_raise(%u) returned: %s\n",
		SHELL_SIGNAL_TEST, signal_status_name(status));

	printk("[raise] raise %u (unregistered): %s\n", SHELL_SIGNAL_UNREGISTERED,
		signal_status_name(signal_raise(SHELL_SIGNAL_UNREGISTERED)));
	printk("[raise] raise %u (out of range): %s\n", SIGNAL_COUNT,
		signal_status_name(signal_raise(SIGNAL_COUNT)));
}

static void schedule_demo()
{
	uint32_t now = timer_ticks();
	printk("[schedule] schedule %u in %u ms at tick %u (due at tick %u): %s\n",
		SHELL_SIGNAL_TEST, SIGNAL_TEST_DELAY_MS, now,
		now + SIGNAL_TEST_DELAY_MS / TIMER_MS_PER_TICK,
		signal_status_name(signal_schedule(SHELL_SIGNAL_TEST, SIGNAL_TEST_DELAY_MS)));

	//
	// Fill the remaining slots of the queue with silent signals, then one more
	//
	uint32_t        scheduled = 0;
	signal_status_t status;
	while ((status = signal_schedule(SHELL_SIGNAL_COUNT, 0)) == SIGNAL_SUCCESS)
		scheduled++;
	printk("[schedule] filling the queue: %u more, then %s (%u dropped in total)\n",
		scheduled, signal_status_name(status), signal_dropped_count());
}

//
// Traps: the CPU saves the address of the next instruction, the exception
// handler queues a signal and returns there, so the command goes on. The signal
// is delivered by the main loop once the command has returned.
//
static void int3_demo()
{
	printk("[trap] int $3: the breakpoint handler queues SIGTRAP (%u)\n",
		SIGNAL_TRAP);
	__asm__ volatile ("int $3");
	printk("[trap] resumed after int $3, SIGTRAP comes after the prompt\n");
}

static void into_demo()
{
	printk("[trap] into with OF set: the overflow handler queues SIGSEGV (%u)\n",
		SIGNAL_SEGMENTATION_FAULT);
	__asm__ volatile ("mov $0x7fffffff, %%eax; add $1, %%eax; into"
		: : : "eax", "cc");
	printk("[trap] resumed after into, SIGSEGV comes after the prompt\n");
}

//
// Faults: the kernel can't fix their cause, so the handler shows the panic
// screen and halts the CPU. These messages are replaced by the panic screen.
//
static void div0_demo()
{
	volatile int zero = 0;

	printk("[fault] dividing by zero: the kernel panics\n");
	printk("%d\n", 1 / zero);
}

static void gpf_demo()
{
	//
	// 0x1234 selects an entry of the LDT, which doesn't exist: the CPU raises
	// a general protection fault with the selector as error code
	//
	printk("[fault] loading selector 0x1234 into DS: the kernel panics\n");
	__asm__ volatile ("mov $0x1234, %%ax; mov %%ax, %%ds" : : : "eax");
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
	shell.executing    = false;

	signal_register(SHELL_SIGNAL_TEST,  &on_test_signal);
	signal_register(SHELL_SIGNAL_COUNT, &on_count_signal);
	signal_register(SIGNAL_TRAP, &on_test_signal);
	signal_register(SIGNAL_SEGMENTATION_FAULT, &on_test_signal);

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
	add_shell_cmd(10, "raise",     "Signal: deliver signal 1 now",          &raise_demo);
	add_shell_cmd(11, "schedule",  "Signal: deliver signal 1 in 1 second",  &schedule_demo);
	add_shell_cmd(12, "int3",      "Trap: int $3, SIGTRAP (5), resume",     &int3_demo);
	add_shell_cmd(13, "into",      "Trap: overflow, SIGSEGV (11), resume",  &into_demo);
	add_shell_cmd(14, "div0",      "Fault: divide by zero, kernel panic",   &div0_demo);
	add_shell_cmd(15, "gpf",       "Fault: bad DS selector, kernel panic",  &gpf_demo);
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

			shell.executing = true;
			shell.commands[i].function();
			shell.executing = false;

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
