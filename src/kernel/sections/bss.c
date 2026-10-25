#include "terminal.h"

extern unsigned char bss_start[];
extern unsigned char bss_end[];

void check_bss()
{
	terminal_begin_output();

	terminal_putnbr_output(bss_end - bss_start);
	terminal_putc_output('\n');

    for (unsigned char *p = bss_start; p < bss_end; ++p)
    {
        terminal_putnbr_output(*p);
    }
	terminal_end_output(true);
}
