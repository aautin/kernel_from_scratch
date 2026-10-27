#pragma once

#include <stdint.h>

enum interrupt_vector_index
{
    //
    // CPU exceptions (vectors 0x00-0x1F)
    //
    INTERRUPT_VECTOR_DIVISION_BY_ZERO              = 0x00,
    INTERRUPT_VECTOR_DEBUGGER                      = 0x01,
    INTERRUPT_VECTOR_NMI                           = 0x02,
    INTERRUPT_VECTOR_BREAKPOINT                    = 0x03,
    INTERRUPT_VECTOR_OVERFLOW                      = 0x04,
    INTERRUPT_VECTOR_BOUNDS                        = 0x05,
    INTERRUPT_VECTOR_INVALID_OPCODE                = 0x06,
    INTERRUPT_VECTOR_COPROCESSOR_NOT_AVAILABLE     = 0x07,
    INTERRUPT_VECTOR_DOUBLE_FAULT                  = 0x08, // Error code (always 0)
    INTERRUPT_VECTOR_COPROCESSOR_SEGMENT_OVERRUN   = 0x09, // 386 or earlier only
    INTERRUPT_VECTOR_INVALID_TASK_STATE_SEGMENT    = 0x0A, // Error code
    INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT           = 0x0B, // Error code
    INTERRUPT_VECTOR_STACK_FAULT                   = 0x0C, // Error code
    INTERRUPT_VECTOR_GENERAL_PROTECTION_FAULT      = 0x0D, // Error code
    INTERRUPT_VECTOR_PAGE_FAULT                    = 0x0E, // Error code
    INTERRUPT_VECTOR_RESERVED                      = 0x0F, // Never raised by the CPU
    INTERRUPT_VECTOR_MATH_FAULT                    = 0x10,
    INTERRUPT_VECTOR_ALIGNMENT_CHECK               = 0x11, // Error code
    INTERRUPT_VECTOR_MACHINE_CHECK                 = 0x12,
    INTERRUPT_VECTOR_SIMD_FLOATING_POINT_EXCEPTION = 0x13,

    //
    // IRQs (vectors 0x20-0x2F)
    //
    INTERRUPT_VECTOR_KEYBOARD = 0x21, // IRQ1
    INTERRUPT_VECTOR_MOUSE    = 0x2C, // IRQ12
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
