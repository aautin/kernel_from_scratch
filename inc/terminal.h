#pragma once

#include <stdint.h>

void switch_color_scheme();
void switch_screen();

void put_screen();

void putc(char c);
void putnbr(int32_t number);
void puts(const char* message);
void del(uint32_t count);
