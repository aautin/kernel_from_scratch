#include <stdint.h>
#include <stdbool.h>

#include "stack.h"
#include "printk.h"
#include "char.h"

#define STACK_DUMP_BYTES     256
#define STACK_DUMP_WORDS     16
#define STACK_RETURN_SEARCH  64
#define STACK_STRING_MIN_LEN 4

extern uint32_t stack_top;

//
// Defined by the linker script: the bounds of the kernel code.
//
extern uint32_t text_start;
extern uint32_t text_end;

//
// A value in the kernel code is probably a return address.
//
static bool is_code_address(uint32_t value)
{
	return value >= (uint32_t) &text_start && value < (uint32_t) &text_end;
}

static void print_return_addresses(uint32_t esp, uint32_t stack_base)
{
	printk("\nLooking for return addresses...\n");
	for (uint32_t* ptr = (uint32_t*) esp;
		(uint32_t) ptr < stack_base && (uint32_t) ptr < esp + STACK_RETURN_SEARCH; ptr++)
	{
		if (is_code_address(*ptr))
			printk("Possible return address at 0x%x: 0x%x\n", (uint32_t) ptr, *ptr);
	}
}

static void print_bytes(uint32_t esp, uint32_t ebp, uint32_t stack_base)
{
	const uint8_t* bytes = (const uint8_t*) esp;

	printk("\n=== Detailed Stack Contents (with ASCII) ===\n");
	printk("Addr       Hex Value     ASCII           Notes\n");
	printk("---------- ------------  --------------  ----------\n");

	for (uint32_t offset = 0; offset < STACK_DUMP_BYTES; offset += 16)
	{
		uint32_t address = esp + offset;
		if (address >= stack_base)
			break;
		printk("0x%08x  ", address);

		for (uint32_t i = 0; i < 16; i++)
		{
			if (address + i < stack_base)
				printk("%02x ", bytes[offset + i]);
			else
				printk("   ");
			if (i == 7)
				printk(" ");
		}
		printk(" ");

		for (uint32_t i = 0; i < 16; i++)
		{
			if (address + i < stack_base)
				printk("%c", isprint(bytes[offset + i]) ? bytes[offset + i] : '.');
			else
				printk(" ");
		}

		printk("  ");
		if (address == esp)
			printk("[ESP] ");
		if (address == ebp)
			printk("[EBP] ");
		if (address == ebp + 4)
			printk("[Ret Addr] ");

		const char* str = (const char*) address;
		int32_t length = 0;
		while (length < 16 && address + length < stack_base && isprint(str[length]))
			length++;
		if (length >= STACK_STRING_MIN_LEN && str[length] == '\0')
		{
			printk("\"");
			for (int32_t i = 0; i < length; i++)
				printk("%c", str[i]);
			printk("\" ");
		}
		printk("\n");
	}
}

static void print_words(uint32_t esp, uint32_t ebp, uint32_t stack_base)
{
	printk("\n=== Word View (for stack frames) ===\n");
	printk("Addr       Value        Notes\n");
	printk("---------- -----------  --------------------\n");

	const uint32_t* word = (const uint32_t*) esp;
	for (uint32_t i = 0; i < STACK_DUMP_WORDS && (uint32_t) word < stack_base; i++, word++)
	{
		uint32_t address = (uint32_t) word;
		uint32_t value   = *word;

		char ascii[5];
		for (uint32_t j = 0; j < 4; j++)
		{
			char c = (char) ((value >> (j * 8)) & 0xFF);
			ascii[j] = isprint(c) ? c : '.';
		}
		ascii[4] = '\0';

		printk("0x%08x  0x%08x  ", address, value);

		if (address == esp)
			printk("[ESP] ");
		else if (address == ebp)
			printk("[EBP] ");
		else if (value == ebp)
			printk("[Saved EBP from 0x%x] ", address - 4);
		else if (is_code_address(value))
			printk("[Code ~0x%x] ", value);

		printk("'%s'\n", ascii);
	}
}

void stack_dump(uint32_t esp, uint32_t ebp)
{
	uint32_t stack_base = (uint32_t) &stack_top;

	printk("=== Kernel Stack Dump ===\n");
	printk("ESP: 0x%x | EBP: 0x%x | Used: %d bytes\n\n", esp, ebp, stack_base - esp);
	printk("Stack frame size: %d bytes\n", ebp - esp);

	print_return_addresses(esp, stack_base);
	print_bytes(esp, ebp, stack_base);
	print_words(esp, ebp, stack_base);
}
