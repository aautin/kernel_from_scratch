#include <stdint.h>

static uint32_t write_index = 0;

void write_debug_message(const char* message)
{
	volatile uint16_t* video_memory = (volatile uint16_t*) 0xB8000;

	for (uint32_t index = 0; message[index] != '\0'; index++)
	{
		if (write_index >= 80 * 25)
		{
			for (uint32_t i = 0; i < 80 * 25; i++)
			{
				video_memory[i] = (uint16_t) ' ' | 0x0F00;
			}

			write_index = 0;
		}

		if (message[index] == '\n')
		{
			write_index += 80 - (write_index % 80);
			continue;
		}

		video_memory[write_index] = (uint16_t) message[index] | 0x0F00;
		write_index++;
	}
}
