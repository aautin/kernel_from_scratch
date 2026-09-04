#include "lib.h"

size_t strlen(const char* str) 
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

int	strcmp(const char *s1, const char *s2)
{
	size_t	i;

	i = 0;
	while ((s1[i] || s2[i]))
	{
		if (s1[i] != s2[i])
			return ((unsigned char)s1[i] - (unsigned char)s2[i]);
		i++;
	}
	return (0);
}
