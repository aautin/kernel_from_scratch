#include <stdarg.h>

#include "terminal.h"
#include "string.h"

uint64_t putchar(char c)
{
	terminal_putc_output(c);
	return 1;
}

uint64_t putstr(const char* s)
{
	terminal_puts_output(s);
	return strlen(s);
}

uint32_t putnbr(int32_t number)
{
	uint32_t count = 0;

	if (number == 0)
	{
		terminal_putc_output('0');
		return 1;
	}

	if (number < 0)
	{
		terminal_putc_output('-');
		count++;
		uint32_t absolute_number = (uint32_t) (-(number + 1)) + 1;
		char digits[20];
		int index = 0;

		while (absolute_number > 0)
		{
			digits[index++] = (char) ('0' + (absolute_number % 10));
			absolute_number /= 10;
		}

		for (int i = index - 1; i >= 0; i--)
		{
			terminal_putc_output(digits[i]);
			count++;
		}

		return count;
	}

	uint32_t absolute_number = (uint32_t) number;
	char digits[20];
	int index = 0;

	while (absolute_number > 0)
	{
		digits[index++] = (char) ('0' + (absolute_number % 10));
		absolute_number /= 10;
	}

	for (int i = index - 1; i >= 0; i--)
	{
		terminal_putc_output(digits[i]);
		count++;
	}

	return count;
}

uint64_t puthex(uint64_t number)
{
    uint64_t count = 0;
    const char *digits = "0123456789abcdef";

    if (number == 0)
    {
        terminal_putc_output('0');
        return 1;
    }

    char hex_digits[16];
    int index = 0;

    while (number)
    {
        hex_digits[index++] = digits[number & 0xf];
        number >>= 4;
    }

    while (index--)
    {
        terminal_putc_output(hex_digits[index]);
        count++;
    }

    return count;
}

uint32_t putptr(void* ptr)
{
	uint32_t address = (uint32_t) ptr;
	return putstr("0x") + puthex(address);
}

uint64_t printk(const char *s, ...)
{
	va_list args;
	uint64_t count;

	va_start(args, s);
	terminal_begin_output();

	count = 0;
	for (uint64_t i = 0; s[i]; i++)
	{
		if (s[i] == '%')
		{
			int has_long_long = 0;
			i++;
			if (s[i] == 'l' && s[i + 1] == 'l')
			{
				has_long_long = 1;
				i += 2;
			}
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
					if (has_long_long)
						count += puthex(va_arg(args, uint64_t));
					else
						count += puthex(va_arg(args, unsigned int));
					break;
				case 'p':
					count += putptr(va_arg(args, void *));
					break;
				default:
					count += putchar(s[i]);
					break;
			}
		}
		else
		{
			count += putchar(s[i]);
		}
	}
	va_end(args);
	terminal_end_output();
	
	return count;
}
