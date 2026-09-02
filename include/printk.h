#ifndef PRINTK_H
#define PRINTK_H

#include "lib.h"
#include "tty.h"
#include <stdarg.h>

int printk(const char* format, ...);

#endif