#include <stdint.h>
#include <stdbool.h>

#include "mouse.h"
#include "keyboard.h"
#include "port.h"

//
// Bounded waits, so the boot doesn't hang when there is no mouse
//
#define PS2_TIMEOUT 10000
#define PS2_RETRIES 3

enum ps2_port
{
	PS2_DATA_PORT    = 0x60,
	PS2_STATUS_PORT  = 0x64,
	PS2_COMMAND_PORT = 0x64,
};

enum ps2_status
{
	PS2_STATUS_OUTPUT_FULL = 1 << 0, // A byte can be read from the data port
	PS2_STATUS_INPUT_FULL  = 1 << 1, // The controller hasn't read the last byte yet
	PS2_STATUS_FROM_MOUSE  = 1 << 5, // The byte to read comes from the mouse
};

enum ps2_command
{
	PS2_READ_CONFIG       = 0x20,
	PS2_WRITE_CONFIG      = 0x60,
	PS2_DISABLE_MOUSE     = 0xA7,
	PS2_ENABLE_MOUSE      = 0xA8,
	PS2_DISABLE_KEYBOARD  = 0xAD,
	PS2_ENABLE_KEYBOARD   = 0xAE,
	PS2_WRITE_TO_MOUSE    = 0xD4,
};

enum ps2_config
{
	PS2_CONFIG_MOUSE_IRQ   = 1 << 1, // Raise IRQ12 when the mouse sends a byte
	PS2_CONFIG_MOUSE_CLOCK = 1 << 5, // Set when the mouse clock is disabled
};

enum mouse_command
{
	MOUSE_GET_ID          = 0xF2,
	MOUSE_SET_SAMPLE_RATE = 0xF3,
	MOUSE_ENABLE_DATA     = 0xF4,
	MOUSE_SET_DEFAULTS    = 0xF6,
	MOUSE_ACK             = 0xFA,

	//
	// A mouse answers this ID after the sample rates 200, 100, 80 when it has
	// a scroll wheel. Its packets then have a 4th byte for the wheel.
	//
	MOUSE_ID_WHEEL = 3,
};

//
// Bit always set in the first byte of a packet, used to stay in sync
//
#define MOUSE_PACKET_ALWAYS_ONE 0x08

//
// Same scancodes as the Up and Down arrows
//
#define MOUSE_WHEEL_UP_SCANCODE   0x48
#define MOUSE_WHEEL_DOWN_SCANCODE 0x50

static bool has_scroll_wheel = false;

static void wait_input_empty()
{
	for (uint32_t i = 0; i < PS2_TIMEOUT && (inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL); i++)
		io_wait();
}

static bool wait_output_full()
{
	for (uint32_t i = 0; i < PS2_TIMEOUT; i++)
	{
		if (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL)
			return true;
		io_wait();
	}
	return false;
}

static bool mouse_write(uint8_t data)
{
	for (uint32_t retry = 0; retry < PS2_RETRIES; retry++)
	{
		wait_input_empty();
		outb(PS2_COMMAND_PORT, PS2_WRITE_TO_MOUSE);
		wait_input_empty();
		outb(PS2_DATA_PORT, data);

		if (wait_output_full() && inb(PS2_DATA_PORT) == MOUSE_ACK)
			return true;
	}
	return false;
}

void mouse_init()
{
	//
	// Disable both devices and drop any pending byte
	//
	outb(PS2_COMMAND_PORT, PS2_DISABLE_MOUSE);
	outb(PS2_COMMAND_PORT, PS2_DISABLE_KEYBOARD);
	inb(PS2_DATA_PORT);

	//
	// Only the two mouse bits change, the keyboard settings are kept
	//
	wait_input_empty();
	outb(PS2_COMMAND_PORT, PS2_READ_CONFIG);
	wait_output_full();
	uint8_t config = inb(PS2_DATA_PORT);
	config |= PS2_CONFIG_MOUSE_IRQ;
	config &= ~PS2_CONFIG_MOUSE_CLOCK;
	wait_input_empty();
	outb(PS2_COMMAND_PORT, PS2_WRITE_CONFIG);
	wait_input_empty();
	outb(PS2_DATA_PORT, config);

	outb(PS2_COMMAND_PORT, PS2_ENABLE_KEYBOARD);
	outb(PS2_COMMAND_PORT, PS2_ENABLE_MOUSE);

	if (!mouse_write(MOUSE_SET_DEFAULTS))
	{
		//
		// No mouse
		//
		return;
	}

	mouse_write(MOUSE_SET_SAMPLE_RATE);
	mouse_write(200);
	mouse_write(MOUSE_SET_SAMPLE_RATE);
	mouse_write(100);
	mouse_write(MOUSE_SET_SAMPLE_RATE);
	mouse_write(80);
	mouse_write(MOUSE_GET_ID);
	has_scroll_wheel = wait_output_full() && inb(PS2_DATA_PORT) == MOUSE_ID_WHEEL;

	mouse_write(MOUSE_ENABLE_DATA);
}

void mouse_interrupt_handler(interrupt_frame_t* frame)
{
	(void) frame;

	static uint8_t cycle = 0;
	static uint8_t packet[4];

	uint8_t status = inb(PS2_STATUS_PORT);
	if (!(status & PS2_STATUS_OUTPUT_FULL) || !(status & PS2_STATUS_FROM_MOUSE))
	{
		//
		// No mouse byte to read, e.g. an IRQ left pending by mouse_init()
		//
		return;
	}

	packet[cycle] = inb(PS2_DATA_PORT);
	if (cycle == 0 && !(packet[0] & MOUSE_PACKET_ALWAYS_ONE))
	{
		//
		// Not the first byte of a packet, wait for the next one
		//
		return;
	}

	cycle++;
	if (cycle < (has_scroll_wheel ? 4 : 3))
		return;
	cycle = 0;

	if (has_scroll_wheel)
	{
		int8_t wheel = (int8_t) packet[3];
		if (wheel < 0)
			keyboard_push_scancode(MOUSE_WHEEL_UP_SCANCODE);
		else if (wheel > 0)
			keyboard_push_scancode(MOUSE_WHEEL_DOWN_SCANCODE);
	}
}
