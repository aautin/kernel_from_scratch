#pragma once

#include <stdint.h>

enum gdt_selector_index
{
	NULL_I        = 0,
	KERNEL_CODE_I = 1,
	KERNEL_DATA_I = 2,
	USER_CODE_I   = 3,
	USER_DATA_I   = 4,
};
typedef enum gdt_selector_index selector_i;

enum gdt_selector_offset
{
	NULL_OFFSET        = 0x00,
	KERNEL_CODE_OFFSET = 0x08,
	KERNEL_DATA_OFFSET = 0x10,
	USER_CODE_OFFSET   = 0x18,
	USER_DATA_OFFSET   = 0x20,
};
typedef enum gdt_selector_offset selector_offset;

enum gdt_segment_range
{
	ADDRESS_MIN = 0x00000000,
	ADDRESS_MAX = 0xFFFFFFFF,
};

enum gdt_access_byte
{
	PRESENT      = 0x80,
	
	PRIVILEGE_0  = 0x00,
	PRIVILEGE_3  = 0x60,

	DESCRIPTOR  = 0x10, // 0=system, 1=code or data segment
	
	EXECUTABLE   = 0x08,

	DIRECTION    = 0x04, // Data segments: 0=expand-up, 1=expand-down.
						 // Code segments: 0=conforming, 1=non-conforming.
	
	READ_WRITE   = 0x02, // Data segments: 0=read-only, 1=read/write.
						 // Code segments: 0=execute-only, 1=execute/read.

	ACCESSED     = 0x01,
};

struct gdt_entry
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t  base_middle;
	uint8_t  access;
	uint8_t  granularity;
	uint8_t  base_high;
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

struct gdt_ptr
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));
typedef struct gdt_ptr gdt_ptr_t;

void register_gdt();
