#pragma once

#include <stdint.h>

enum gdt_selector_index
{
	NULL_I,
	KERNEL_CODE_I,
	KERNEL_DATA_I,
	KERNEL_STACK_I,
	USER_CODE_I,
	USER_DATA_I,
	USER_STACK_I,
};
typedef enum gdt_selector_index selector_i;

enum gdt_selector_offset
{ 
	NULL_OFFSET         = 0x00,
	KERNEL_CODE_OFFSET  = 0x08,
	KERNEL_DATA_OFFSET  = 0x10,
	KERNEL_STACK_OFFSET = 0x18,
	USER_CODE_OFFSET    = 0x20,
	USER_DATA_OFFSET    = 0x28,
	USER_STACK_OFFSET   = 0x30,
};
typedef enum gdt_selector_offset selector_offset;

enum gdt_segment_range
{
	ADDRESS_MIN = 0x00000000,
	ADDRESS_MAX = 0xFFFFFFFF,
};

enum gdt_access_byte
{
	PRESENT  = 0x80, // Always set to 1 (meaning the segment is valid).
	ACCESSED = 0x01, // Always set to 0, the CPU sets to 1 when accessing it.
	
	PRIVILEGE_0 = 0x00,
	PRIVILEGE_3 = 0x60,
	
	DESCRIPTOR = 0x10, // 0=system (e.g., TSS, LDT), 1=code or data segment.
	
	EXECUTABLE = 0x08,
	DIRECTION  = 0x04, // Executable     : 0=expand-up,  1=expand-down.
					   // Non-executable : 0=conforming, 1=non-conforming.
	READ_WRITE = 0x02, // Executable     : 0=execute-only, 1=readable.
					   // Non-executable : 0=read-only,    1=read/write.
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
