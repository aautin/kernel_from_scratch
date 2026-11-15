#pragma once

#include <stdint.h>

#include "interrupt.h"

//
// The PIT raises IRQ0 TIMER_FREQUENCY times per second.
//
#define TIMER_FREQUENCY   100
#define TIMER_MS_PER_TICK (1000 / TIMER_FREQUENCY)

//
// Program the PIT channel 0. Called while interrupts are still disabled.
//
void timer_init();

void timer_interrupt_handler(interrupt_frame_t* frame);

//
// Number of ticks since timer_init(). Wraps around after about 497 days.
//
uint32_t timer_ticks();
