#include <stdbool.h>

bool islower(char c)
{
	return (c >= 'a' && c <= 'z');
}

bool isupper(char c)
{
	return (c >= 'A' && c <= 'Z');
}

bool isalpha(char c)
{
	return (islower(c) || isupper(c));
}

bool isdigit(char c)
{
	return (c >= '0' && c <= '9');
}

bool isalnum(char c)
{
	return (isalpha(c) || isdigit(c));
}

bool isprint(char c)
{
	return (c >= 32 && c <= 126);
}

char tolower(char c)
{
	if (isupper(c))
	{
		return c + 32;
	}
	return c;
}

char toupper(char c)
{
	if (islower(c))
	{
		return c - 32;
	}
	return c;
}
