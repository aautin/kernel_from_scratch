#pragma once

#include <stdint.h>
#include <stdbool.h>

//
// Signals are numbered from 1 to SIGNAL_COUNT - 1, 0 is never a valid signal.
//
#define SIGNAL_COUNT      32
#define SIGNAL_QUEUE_SIZE 32

typedef void (*signal_handler_t)(uint8_t number);

enum signal_status
{
	SIGNAL_SUCCESS = 0,
	SIGNAL_ERROR_INVALID_NUMBER, // The number is 0 or not below SIGNAL_COUNT
	SIGNAL_ERROR_NOT_REGISTERED, // No function is registered for the number
	SIGNAL_ERROR_QUEUE_FULL,     // The signal is dropped and counted
};
typedef enum signal_status signal_status_t;

//
// Set the function called when the signal is delivered. A null function
// unregisters the signal: its scheduled deliveries are then dropped.
//
signal_status_t signal_register(uint8_t number, signal_handler_t function);

//
// Deliver the signal now: its function is called before signal_raise returns.
//
// Callbacks never run inside an interrupt, nor inside another callback (to keep
// the stack bounded): from there, the signal is queued like signal_schedule
// with no delay, and delivered by the main loop.
//
signal_status_t signal_raise(uint8_t number);

//
// Queue the signal, to be delivered by the main loop once delay_ms milliseconds
// have passed (rounded up to the timer resolution). Safe to call from an
// interrupt handler.
//
// Due signals are delivered in the order of their due time, then in the order
// they were scheduled.
//
signal_status_t signal_schedule(uint8_t number, uint32_t delay_ms);

//
// Called by the main loop: delivers the due signals. Signals queued by the
// callbacks themselves wait for the next call, so it always returns.
//
void signal_handle_pending();

//
// True if a queued signal is due. Used by the main loop before 'hlt'.
//
bool signal_has_pending();

//
// Number of signals dropped because the queue was full.
//
uint32_t signal_dropped_count();
