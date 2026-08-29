#pragma once

#include <stdint.h>

//
// I/O on ports
//
void    outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void    io_wait();

//
// I/O on VGA
//
void    vga_remove_last_character();
void    vga_putc(uint16_t c);
void    vga_putnbr(int32_t number);
void    vga_puts(const char* message);
void    vga_put_screen();
void    vga_switch_next_screen();
void    vga_switch_color();
