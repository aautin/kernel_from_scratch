NAME     := kfs.elf

CC       := gcc
AS       := nasm
LD       := ld

CDEFAULTFLAGS := -Wall -Wextra -Werror
CFLAGS        := $(CDEFAULTFLAGS) -ffreestanding -fno-builtin -nostdlib -nodefaultlibs -fno-stack-protector -fno-pie -fno-pic -m32 -g
ASFLAGS       := -f elf32
LDFLAGS       := -m elf_i386 -nostdlib -Map=kernel.map

INC_PATH := inc inc/helper inc/io
SRC_PATH := src
OBJ_PATH := obj

LD_SCRIPT := $(SRC_PATH)/misc/linker.ld

C_SRC_FILES  := kernel/boot/main \
				kernel/boot/multiboot \
				\
				kernel/gdt \
				\
				kernel/interrupt/idt \
				kernel/interrupt/pic \
				kernel/interrupt/interrupt \
				kernel/interrupt/handler/keyboard \
				kernel/interrupt/handler/mouse \
				\
				kernel/io/vga \
				kernel/io/port \
				\
				kernel/sections/bss \
				\
				kernel/terminal \
				kernel/shell \
				kernel/stack \
				\
				kernel/helper/char \
				kernel/helper/printk \
				kernel/helper/mem \
				kernel/helper/string

C_SRC        := $(addsuffix .c, $(addprefix $(SRC_PATH)/, $(C_SRC_FILES)))

AS_SRC_FILES := kernel/boot/entry \
				\
				kernel/sections/multiboot \
				kernel/sections/gdt \
				\
				kernel/interrupt/handler/irq_stub \
				kernel/interrupt/handler/exception_stub

AS_SRC       := $(addsuffix .asm, $(addprefix $(SRC_PATH)/, $(AS_SRC_FILES)))

SRC := $(C_SRC) $(AS_SRC)

C_OBJS  := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(C_SRC_FILES)))
AS_OBJS := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(AS_SRC_FILES)))
INC_ARGS := $(addprefix -I, $(INC_PATH))

ISO_PATH := iso
ISO_FILE := bootable_kernel.iso
ISO      := $(ISO_PATH)/$(ISO_FILE)

GRUB_CFG := $(SRC_PATH)/misc/grub.cfg

DOCKERFILE   := $(SRC_PATH)/misc/Dockerfile
DOCKER_IMAGE := kfs-builder
DOCKER_CMD   := grub-mkrescue --compress=xz --modules="multiboot normal" --locales="" --fonts="" -o $(ISO) boot/$(NAME)=$(NAME) boot/grub/grub.cfg=$(GRUB_CFG)

.PHONY: all clean fclean re run

all: $(ISO)

$(NAME): $(C_OBJS) $(AS_OBJS)
	$(LD) $(LDFLAGS) -T $(LD_SCRIPT) -o $(NAME) $(C_OBJS) $(AS_OBJS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.asm
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(INC_ARGS) $< -o $@

$(ISO): $(NAME) $(GRUB_CFG)
	docker build -t $(DOCKER_IMAGE) -f $(DOCKERFILE) .
	mkdir -p $(dir $@)
	docker run --rm --user $(shell id -u):$(shell id -g) -v "$(CURDIR):/workspace" -w /workspace $(DOCKER_IMAGE) $(DOCKER_CMD)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -rf $(OBJ_PATH)

fclean: clean
	rm -f $(NAME)

re: fclean all
