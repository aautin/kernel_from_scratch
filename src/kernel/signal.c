#include <stdint.h>
#include <stdbool.h>

#include "signal.h"
#include "interrupt.h"
#include "timer.h"

#define EFLAGS_INTERRUPT_FLAG (1 << 9)

//
// The queue is written by the interrupt handlers and the main loop, so it is
// only accessed with interrupts disabled. Its entries have different due
// times, so the free slots are searched instead of using a ring buffer.
//
struct signal_entry
{
	bool     used;
	uint8_t  number;
	uint32_t due_tick;
	uint32_t sequence; // Scheduling order, used to deliver the ties in order
};
typedef struct signal_entry signal_entry_t;

static signal_handler_t handlers[SIGNAL_COUNT];
static signal_entry_t   queue[SIGNAL_QUEUE_SIZE];
static uint32_t         next_sequence = 0;
static uint32_t         dropped = 0;

//
// Number of callbacks being executed, a raise from one of them is queued
//
static uint32_t delivery_depth = 0;

//
// Disable interrupts, and return the previous flags to restore them: the
// functions can also be called from an interrupt, where they are already
// disabled and must stay so.
//
static uint32_t lock()
{
	uint32_t flags;
	__asm__ volatile ("pushf; pop %0; cli" : "=r"(flags) : : "memory");
	return flags;
}

static void unlock(uint32_t flags)
{
	if (flags & EFLAGS_INTERRUPT_FLAG)
		__asm__ volatile ("sti" : : : "memory");
}

static bool is_valid(uint8_t number)
{
	return number > 0 && number < SIGNAL_COUNT;
}

//
// Tick counters wrap around, so they are compared through their difference
//
static bool is_before(uint32_t a, uint32_t b)
{
	return (int32_t) (a - b) < 0;
}

static bool is_due(const signal_entry_t* entry, uint32_t now)
{
	return !is_before(now, entry->due_tick);
}

static void deliver(uint8_t number, signal_handler_t handler)
{
	delivery_depth++;
	handler(number);
	delivery_depth--;
}

signal_status_t signal_register(uint8_t number, signal_handler_t function)
{
	if (!is_valid(number))
		return SIGNAL_ERROR_INVALID_NUMBER;

	uint32_t flags = lock();
	handlers[number] = function;
	unlock(flags);

	return SIGNAL_SUCCESS;
}

signal_status_t signal_raise(uint8_t number)
{
	if (!is_valid(number))
		return SIGNAL_ERROR_INVALID_NUMBER;

	if (interrupt_is_active() || delivery_depth > 0)
		return signal_schedule(number, 0);

	signal_handler_t handler = handlers[number];
	if (!handler)
		return SIGNAL_ERROR_NOT_REGISTERED;

	deliver(number, handler);
	return SIGNAL_SUCCESS;
}

signal_status_t signal_schedule(uint8_t number, uint32_t delay_ms)
{
	if (!is_valid(number))
		return SIGNAL_ERROR_INVALID_NUMBER;

	//
	// Round up, so the signal never comes before the delay
	//
	uint32_t delay_ticks = delay_ms / TIMER_MS_PER_TICK
						 + (delay_ms % TIMER_MS_PER_TICK != 0);

	signal_status_t status = SIGNAL_ERROR_QUEUE_FULL;
	uint32_t flags = lock();

	if (!handlers[number])
	{
		status = SIGNAL_ERROR_NOT_REGISTERED;
	}
	else
	{
		for (uint32_t i = 0; i < SIGNAL_QUEUE_SIZE; i++)
		{
			if (queue[i].used)
				continue;

			queue[i].used     = true;
			queue[i].number   = number;
			queue[i].due_tick = timer_ticks() + delay_ticks;
			queue[i].sequence = next_sequence++;
			status = SIGNAL_SUCCESS;
			break;
		}

		if (status == SIGNAL_ERROR_QUEUE_FULL)
			dropped++;
	}

	unlock(flags);
	return status;
}

void signal_handle_pending()
{
	//
	// Only the signals scheduled before this call are delivered, so a callback
	// scheduling itself again can't keep the main loop here forever
	//
	uint32_t flags = lock();
	uint32_t end_sequence = next_sequence;
	unlock(flags);

	while (true)
	{
		flags = lock();

		uint32_t        now  = timer_ticks();
		signal_entry_t* next = 0;
		for (uint32_t i = 0; i < SIGNAL_QUEUE_SIZE; i++)
		{
			signal_entry_t* entry = &queue[i];
			if (!entry->used || !is_due(entry, now) || !is_before(entry->sequence, end_sequence))
				continue;

			if (!next
				|| is_before(entry->due_tick, next->due_tick)
				|| (entry->due_tick == next->due_tick && is_before(entry->sequence, next->sequence)))
			{
				next = entry;
			}
		}

		if (!next)
		{
			unlock(flags);
			return;
		}

		uint8_t          number  = next->number;
		signal_handler_t handler = handlers[number];
		next->used = false;

		unlock(flags);

		//
		// The callback runs with interrupts enabled, outside of any interrupt.
		// A signal unregistered after being scheduled is dropped.
		//
		if (handler)
			deliver(number, handler);
	}
}

bool signal_has_pending()
{
	uint32_t flags = lock();

	bool     pending = false;
	uint32_t now     = timer_ticks();
	for (uint32_t i = 0; i < SIGNAL_QUEUE_SIZE && !pending; i++)
		pending = queue[i].used && is_due(&queue[i], now);

	unlock(flags);
	return pending;
}

uint32_t signal_dropped_count()
{
	return dropped;
}
