#pragma once

#include <stdint.h>
#include <stdarg.h>

uint64_t putchar(char c);
uint64_t putstr(const char* s);
uint32_t putnbr(int32_t number);
uint64_t puthex(uint64_t number);
uint32_t putptr(void* ptr);
uint64_t printk(const char* s, ...);
