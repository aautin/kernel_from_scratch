#ifndef GDT_H
#define GDT_H

#include <stdint.h>

enum segment
{
    NULL_I = 0,
    KERNEL_CODE_I,
    KERNEL_DATA_I,
    USER_CODE_I,
    USER_DATA_I
}; typedef enum segment segment;

enum access_flags
{
    DESCRIPTOR = 0x10,
    PRESENT = 0x80,
    PRIVILEGE_0 = 0x00,
    PRIVILEGE_3 = 0x60,
    EXECUTABLE = 0x08,
    READ_WRITE = 0x02
};

/* Defines a GDT entry. */
struct gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

/* Special pointer which includes the limit: The max bytes
*  taken up by the GDT, minus 1. Again, this NEEDS to be packed */
struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct gdt_ptr gdt_ptr_t;


/* This will be a function in start.asm. We use this to properly
*  reload the new segment registers */
extern void gdt_flush();

void register_gdt(void);

#endif