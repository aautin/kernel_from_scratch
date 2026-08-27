#pragma once

#include <stdint.h>

enum multiboot_flags
{
	MULTIBOOT_INFO_MEMORY  = 0x00000001,
	MULTIBOOT_INFO_MEM_MAP = 0x00000040,
};

struct multiboot_info
{
	uint32_t flags;

	//
	// MULTIBOOT_INFO_MEMORY
	//
	uint32_t mem_lower;
	uint32_t mem_upper;

	//
	// MULTIBOOT_INFO_MEM_MAP
	//
	uint32_t mmap_length;
	uint32_t mmap_addr;
};
