#include "multiboot.h"
#include "printk.h"

static void print_entry(const char* type, uint64_t base, uint64_t length)
{
	uint64_t end = base + length;
	printk("%s memory: 0x%llx - 0x%llx (%d bytes)\n", type, base, end, length);
}

void print_multiboot_info(struct multiboot_info* mbi)
{
	if (mbi->flags & MULTIBOOT_INFO_MEMORY)
	{
		uint32_t lower = mbi->mem_lower;
		uint32_t upper = mbi->mem_upper;
		printk("Lower memory: %d KiB (%d bytes)\n", lower, lower * 1024);
		printk("Upper memory: %d KiB (%d bytes)\n", upper, upper * 1024);
	}

	if (mbi->flags & MULTIBOOT_INFO_MEM_MAP)
	{
		printk("Memory map entries:\n");
		
		uint8_t* current = (uint8_t* )mbi->mmap_addr;
		uint8_t* end     = current + mbi->mmap_length;

		while (current < end)
		{
			multiboot_mmap_entry_t* entry = (multiboot_mmap_entry_t*) current;

			char* type;
			switch (entry->type)
			{
				case MULTIBOOT_MEM_MAP_AVAILABLE:
					type = "Available";
					break;
				case MULTIBOOT_MEM_MAP_RESERVED:
					type = "Reserved";
					break;
				case MULTIBOOT_MEM_MAP_ACPI_RECLAIMABLE:
					type = "ACPI Reclaimable";
					break;
				case MULTIBOOT_MEM_MAP_NVS:
					type = "NVS";
					break;
				case MULTIBOOT_MEM_MAP_BADRAM:
					type = "Bad RAM";
					break;
				default:
					type = "Unknown";
					break;
			}
			print_entry(type, entry->base_addr, entry->length);
			
			current += entry->size + sizeof(entry->size);
		}
	}
}
