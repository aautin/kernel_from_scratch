#pragma once

#include <stdint.h>

enum interrupt_vector_index
{
    INTERRUPT_VECTOR_KEYBOARD = 0x21,
};

enum interrupt_descriptor_flag
{
    //
    // A task gate is used to switch tasks in multitasking operating systems.
    // It allows the CPU to switch from one task to another by loading a new
    // task state segment (TSS) and updating the CPU's registers accordingly.
    //
    // Not used in this kernel version
    //
    INTERUPT_FLAG_TASK_GATE = 0b0101,

    //
    // Interrupt vs trap gates
    //
    // Interrupt: when handling an interrupt,
    // disable further ones to prevent nested interrupts
    //
    // Trap: allow nested interrupts, commonly used to handle CPU
    // exceptions and system calls, where the CPU needs to be able to
    // respond to other interrupts while handling the current one.
    //
    //
    // Gate size: 16-bit vs 32-bit
    //
    INTERUPT_FLAG_16BIT_INTERRUPT_GATE = 0b0110,
    INTERUPT_FLAG_16BIT_GATE           = 0b0111,
    INTERUPT_FLAG_32BIT_INTERRUPT_GATE = 0b1110,
    INTERUPT_FLAG_32BIT_TRAP_GATE      = 0b1111,

    //
    // Privilege level:
    // - Ring 0 is the most privileged (kernel mode)
    // - Ring 3 is the least privileged (user applications)
    //
    INTERRUPT_FLAG_DPL0 = 0 << 5,
    INTERRUPT_FLAG_DPL3 = 3 << 5,

    INTERRUPT_FLAG_PRESENT = 1 << 7
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
