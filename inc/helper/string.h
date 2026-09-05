#pragma once

#include <stdint.h>

uint32_t strlen(const char* str);
uint32_t strcmp(const char* str1, const char* str2);
char*    strncpy(char* destination, const char* source, uint32_t length);
