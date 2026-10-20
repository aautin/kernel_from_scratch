#include <stdarg.h>

#include "terminal.h"
#include "string.h"
#include "char.h"

static uint64_t putchar(char c)
{
	terminal_putc_output(c);
	return 1;
}

static uint64_t putstr(const char* s)
{
	if (!s)
		s = "(null)";
	terminal_puts_output(s);
	return strlen(s);
}

static uint32_t putpadding(uint32_t length, uint32_t width, char pad)
{
	uint32_t count = 0;
	while (length + count < width)
	{
		terminal_putc_output(pad);
		count++;
	}
	return count;
}

static uint32_t nbrlen(uint32_t number)
{
	uint32_t length = 1;
	while (number >= 10)
	{
		number /= 10;
		length++;
	}
	return length;
}

static uint32_t hexlen(uint64_t number)
{
	//
	// Shifts instead of divisions, 64-bit divisions would need libgcc
	//
	uint32_t length = 1;
	while (number >> 4)
	{
		number >>= 4;
		length++;
	}
	return length;
}

static uint32_t putunbr(uint32_t number)
{
	uint32_t count = 0;
	char digits[20];
	int index = 0;

	do
	{
		digits[index++] = (char) ('0' + (number % 10));
		number /= 10;
	}
	while (number > 0);

	for (int i = index - 1; i >= 0; i--)
	{
		terminal_putc_output(digits[i]);
		count++;
	}

	return count;
}

static uint32_t putnbr(int32_t number)
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

static uint64_t puthex(uint64_t number)
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

static uint32_t putptr(void* ptr)
{
	uint32_t address = (uint32_t) ptr;
	uint32_t count = putstr("0x");

	//
	// Addresses always have 8 digits, so they line up in columns
	//
	count += putpadding(hexlen(address), 8, '0');
	count += puthex(address);
	return count;
}

uint64_t printk(const char *s, ...)
{
	va_list args;
	uint64_t count;

	va_start(args, s);

	count = 0;
	for (uint64_t i = 0; s[i]; i++)
	{
		if (s[i] == '%')
		{
			uint64_t start = i;
			int has_long_long = 0;
			char pad = ' ';
			uint32_t width = 0;
			i++;

			//
			// Optional zero-padding and width (e.g. "%08x")
			//
			if (s[i] == '0')
			{
				pad = '0';
				i++;
			}
			while (isdigit(s[i]))
			{
				width = width * 10 + (s[i] - '0');
				i++;
			}
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
				{
					int32_t number = va_arg(args, int);
					uint32_t absolute = number < 0 ? (uint32_t) (-(number + 1)) + 1 : (uint32_t) number;
					uint32_t length = nbrlen(absolute) + (number < 0);

					if (number < 0 && pad == '0')
					{
						//
						// The sign goes before the zeros: "-005", not "00-5"
						//
						count += putchar('-');
						count += putpadding(length, width, pad);
						count += putunbr(absolute);
					}
					else
					{
						count += putpadding(length, width, pad);
						count += putnbr(number);
					}
					break;
				}
				case 'u':
				{
					uint32_t number = va_arg(args, unsigned int);
					count += putpadding(nbrlen(number), width, pad);
					count += putunbr(number);
					break;
				}
				case '%':
					count += putchar('%');
					break;
				case 'x':
				{
					uint64_t number;
					if (has_long_long)
						number = va_arg(args, uint64_t);
					else
						number = va_arg(args, unsigned int);
					count += putpadding(hexlen(number), width, pad);
					count += puthex(number);
					break;
				}
				case 'p':
					count += putptr(va_arg(args, void *));
					break;
				default:
					//
					// Unknown conversion: print it back as written
					//
					for (uint64_t j = start; j <= i; j++)
						count += putchar(s[j]);
					break;
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
