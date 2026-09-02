#pragma once

#include <stdint.h>

enum multiboot_flags
{
	MULTIBOOT_INFO_MEMORY  = 0x00000001,
	MULTIBOOT_INFO_MEM_MAP = 0x00000040,
};
typedef enum multiboot_flags multiboot_flags_t;

struct multiboot_info
{
    uint32_t flags;

    //
	// MULTIBOOT_INFO_MEMORY
	//
	uint32_t mem_lower;
	uint32_t mem_upper;

//----------------- IGNORED -----------------//
    uint32_t boot_device;
    uint32_t cmdline;

    uint32_t mods_count;
    uint32_t mods_addr;

    uint32_t syms[4];
//-------------------------------------------//

    //
	// MULTIBOOT_INFO_MEM_MAP
	//
	uint32_t mmap_length;
	uint32_t mmap_addr;

//----------------- IGNORED -----------------//
    uint32_t drives_length;
    uint32_t drives_addr;

    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;

    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint16_t framebuffer_reserved;
//-------------------------------------------//
};
typedef struct multiboot_info multiboot_info_t;

enum multiboot_mem_map_type
{
	MULTIBOOT_MEM_MAP_AVAILABLE = 1,
	MULTIBOOT_MEM_MAP_RESERVED  = 2,
	MULTIBOOT_MEM_MAP_ACPI_RECLAIMABLE = 3,
	MULTIBOOT_MEM_MAP_NVS = 4,
	MULTIBOOT_MEM_MAP_BADRAM = 5,
};

struct multiboot_mmap_entry
{
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    enum multiboot_mem_map_type type;
};
typedef struct multiboot_mmap_entry multiboot_mmap_entry_t;

void print_multiboot_info(struct multiboot_info* mbi);
