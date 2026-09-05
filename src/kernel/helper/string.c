#include <stdint.h>

uint32_t strlen(const char* str)
{
	uint32_t length = 0;
	while (str[length] != '\0')
	{
		length++;
	}
	return length;
}

uint32_t strcmp(const char* str1, const char* str2)
{
	while (*str1 && (*str1 == *str2))
	{
		str1++;
		str2++;
	}
	return *(const unsigned char*) str1 - *(const unsigned char*) str2;
}

char* strncpy(char* destination, const char* source, uint32_t length)
{
	uint32_t i;
	for (i = 0; i < length && source[i] != '\0'; i++)
	{
		destination[i] = source[i];
	}
	for (; i < length; i++)
	{
		destination[i] = '\0';
	}
	return destination;
}
