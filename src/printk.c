#include "printk.h"

int	ft_putstr(char *s)
{
	if (!s)
	{
		terminal_write("(null)", 6);
		return (6);
	}
    terminal_write(s, strlen(s));
    return (strlen(s));
}

int    ft_putchar(char c)
{
    terminal_putchar(c);
    return (1);
}

static int	ft_nbrlen_hex(unsigned int nbr)
{
	unsigned int	len;

	len = 0;
	if (nbr == 0)
		len++;
	while (nbr > 0)
	{
		nbr /= 16;
		len++;
	}
	return (len);
}

int	ft_print_hex(unsigned int n, char *base)
{
	if (n < 16)
		terminal_write(&base[n], 1);
	else
	{
		ft_print_hex(n / 16, base);
		ft_print_hex(n % 16, base);
	}
	return (ft_nbrlen_hex(n));
}

static int	ft_nbrlen_long(unsigned long nbr)
{
	unsigned long	len;

	len = 0;
	if (nbr == 0)
		len++;
	while (nbr > 0)
	{
		nbr /= 16;
		len++;
	}
	return (len);
}

static int	ft_print_hex_p(unsigned long n, char *base)
{
	if (n < 16)
        terminal_write(&base[n], 1);
	else
	{
		ft_print_hex_p(n / 16, base);
		ft_print_hex_p(n % 16, base);
	}
	return (ft_nbrlen_long(n));
}

int	ft_print_ptr(unsigned long n, char *base)
{
	if (!n)
	{
        terminal_write("(nil)", 5);
		return (5);
	}
    terminal_write("0x", 2);
	n = ft_print_hex_p(n, base);
	return (n + 2);
}

unsigned int	ft_nbrlen(long nbr)
{
	long	len;

	len = 0;
	if (nbr == 0)
		len++;
	if (nbr < 0)
	{
		nbr *= -1;
		len++;
	}
	while (nbr > 0)
	{
		nbr /= 10;
		len++;
	}
	return (len);
}

void	ft_putnbr(int n)
{
	if (n == -2147483648)
        terminal_write("-2147483648", 11);
	else if (n < 0)
	{	
        terminal_write("-", 1);
		n = -n;
		ft_putnbr(n);
	}
	else if (n >= 0 && n <= 9)
		ft_putchar(n + '0');
	else
	{
		ft_putnbr(n / 10);
		ft_putnbr(n % 10);
	}
}

void	ft_unsigned_putnbr(unsigned int n)
{
	if (n < 10)
		ft_putchar(n + '0');
	else
	{
		ft_putnbr(n / 10);
		ft_putnbr(n % 10);
	}
}

unsigned int	ft_printf_numbers_i(int nbr)
{
	ft_putnbr (nbr);
	return (ft_nbrlen(nbr));
}

unsigned int	ft_printf_numbers_u(unsigned int nbr)
{
	ft_unsigned_putnbr (nbr);
	return (ft_nbrlen(nbr));
}


int	ft_arg_format(va_list *args, int i, const char *str)
{	
	if (str[i] == 'c')
		return (ft_putchar(va_arg(*args, int)));
	else if (str[i] == 's')
		return (ft_putstr(va_arg(*args, char *)));
	else if (str[i] == 'd' || str[i] == 'i')
		return (ft_printf_numbers_i(va_arg(*args, int)));
	else if (str[i] == 'u')
		return (ft_printf_numbers_u(va_arg(*args, unsigned int)));
	else if (str[i] == 'X')
		return (ft_print_hex(va_arg(*args, unsigned int), "0123456789ABCDEF"));
	else if (str[i] == 'x')
		return (ft_print_hex(va_arg(*args, unsigned int), "0123456789abcdef"));
	else if (str[i] == 'p')
		return (ft_print_ptr(va_arg(*args, unsigned long), "0123456789abcdef"));
	else if (str[i] == '%')
    {
        terminal_write("%", 1);
		return (1);
    }

	return (0);
}

int	printk(const char *str, ...)
{
	va_list	args;
	int		len_res;
	int		i;

	va_start (args, str);
	len_res = 0;
	i = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '%')
		{
			i++;
			len_res += ft_arg_format(&args, i, str);
		}
		else
			len_res += ft_putchar(str[i]);
		i++;
	}
	va_end (args);
	return (len_res);
}
