#pragma once

#include <stdint.h>

enum interrupt_vector_index
{
    INTERRUPT_VECTOR_KEYBOARD = 0x21,
};

struct interrupt_descriptor_table
{
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));
typedef struct interrupt_descriptor_table idt_t;

struct interrupt_descriptor_table_pointer
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct interrupt_descriptor_table_pointer idt_ptr_t;

void register_interrupt_descriptor_table();
void set_interrupt_descriptor(enum interrupt_vector_index index, uint32_t handler);
void keyboard_interrupt_handler(uint32_t interrupt_number);