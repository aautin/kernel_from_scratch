#include <stdarg.h>

#include "terminal.h"
#include "string.h"

uint32_t putchar(char c)
{
	terminal_putc(c);
	return 1;
}

uint32_t putstr(const char* s)
{
	terminal_puts(s);
	return strlen(s);
}

uint32_t putnbr(int32_t number)
{
	uint32_t count = 0;

	if (number == 0)
	{
		terminal_putc('0');
		return 1;
	}

	if (number < 0)
	{
		terminal_putc('-');
		count++;
		number = -number;
	}

	int32_t reversed_number = 0;
	int32_t digit_count = 0;

	while (number > 0)
	{
		reversed_number = reversed_number * 10 + number % 10;
		number /= 10;
		digit_count++;
	}

	for (int32_t i = 0; i < digit_count; i++)
	{
		terminal_putc((char) ('0' + reversed_number % 10));
		reversed_number /= 10;
		count++;
	}

	return count;
}

uint32_t puthex(uint32_t number)
{
	uint32_t count = 0;
	if (number == 0)
	{
		terminal_putc('0');
		return count + 1;
	}

	char hex_digits[8];
	int index = 0;

	while (number > 0)
	{
		uint32_t digit = number % 16;
		if (digit < 10)
			hex_digits[index++] = '0' + digit;
		else
			hex_digits[index++] = 'a' + (digit - 10);
		number /= 16;
	}

	for (int i = index - 1; i >= 0; i--)
	{
		terminal_putc(hex_digits[i]);
		count++;
	}

	return count;
}

uint32_t putptr(void* ptr)
{
	uint32_t address = (uint32_t) ptr;
	return putstr("0x") + puthex(address);
}

uint32_t printk(const char *s, ...)
{
	va_list	 args;
	uint32_t count;

	va_start(args, s);
	
	count = 0;
	for (uint32_t i = 0; s[i]; i++)
	{
		if (s[i] == '%')
		{
			i++;
			switch (s[i])
			{
				case 'c':
					count += putchar(va_arg(args, int));
					break;
				case 's':
					count += putstr(va_arg(args, char *));
					break;
				case 'd':
				case 'i':
					count += putnbr(va_arg(args, int));
					break;
				case '%':
					count += putchar('%');
					break;
				case 'x':
					count += puthex(va_arg(args, unsigned int));
					break;
				case 'p':
					count += putptr(va_arg(args, void *));
					break;
				default:
					count += putchar(s[i]);
			}
		}
		else
		{
			count += putchar(s[i]);
		}
	}
	va_end(args);

	return count;
}
