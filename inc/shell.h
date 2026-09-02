#pragma once

#include <stdbool.h>

#include "multiboot.h"

void shell_init(multiboot_info_t* mbi);
bool shell_execute(const char* command);
