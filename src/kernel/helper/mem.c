#include <stdint.h>

void* memset(void *dest, int value, uint32_t n)
{
	uint8_t* ptr = (uint8_t*) dest;
	for (uint32_t i = 0; i < n; i++)
	{
		ptr[i] = (uint8_t) value;
	}
	return dest;
}

void* memmove(void *dest, const void *src, uint32_t n)
{
	uint8_t* d = (uint8_t*) dest;
	const uint8_t* s = (const uint8_t*) src;
	
	if (d < s)
	{
		for (uint32_t i = 0; i < n; i++)
		{
			d[i] = s[i];
		}
	}
	else if (d > s)
	{
		for (uint32_t i = n; i > 0; i--)
		{
			d[i - 1] = s[i - 1];
		}
	}
	
	return dest;
}

void* memcpy(void *dest, const void *src, uint32_t n)
{
	return memmove(dest, src, n);
}

int memcmp(const void *s1, const void *s2, uint32_t n)
{
	const uint8_t* p1 = (const uint8_t*) s1;
	const uint8_t* p2 = (const uint8_t*) s2;

	for (uint32_t i = 0; i < n; i++)
	{
		if (p1[i] != p2[i])
		{
			return (int) p1[i] - (int) p2[i];
		}
	}

	return 0;
}
