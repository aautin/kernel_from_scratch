#pragma once

#include <stdint.h>

/*
	PIC Master = IRQs 0-7
	PIC Slave  = IRQs 8-15
*/

#define PIC_MASTER_COMMAND 0x20
#define PIC_SLAVE_COMMAND  0xA0
#define PIC_MASTER_MASK    0x21
#define PIC_SLAVE_MASK     0xA1

#define PIC_MASTER_MASK_IRQ2 0x04
#define PIC_MASTER_MASK_SLAVE PIC_MASTER_MASK_IRQ2
#define PIC_SLAVE_MASK_IRQ2 0x02

#define REMAP_OFFSET_MASTER 0x20
#define REMAP_OFFSET_SLAVE  0x28

void remap_pic();
