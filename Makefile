NAME     := kfs.elf

CC       := gcc
AS       := nasm
LD       := ld

CDEFAULTFLAGS := -Wall -Wextra -Werror
CFLAGS        := $(CDEFAULTFLAGS) -ffreestanding -fno-builtin -nostdlib -m32 -g
ASFLAGS       := -f elf32
LDFLAGS       := -m elf_i386 

INC_PATH := inc
SRC_PATH := src
OBJ_PATH := obj

LD_SCRIPT := linker.ld

C_SRC_FILES  := kernel/main kernel/vga kernel/interrupt kernel/gdt kernel/pic
C_SRC        := $(addsuffix .c, $(addprefix $(SRC_PATH)/, $(C_SRC_FILES)))

AS_SRC_FILES := boot gdt irq_stub
AS_SRC       := $(addsuffix .asm, $(addprefix $(SRC_PATH)/, $(AS_SRC_FILES)))

SRC := $(C_SRC) $(AS_SRC)

C_OBJS  := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(C_SRC_FILES)))
AS_OBJS := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(AS_SRC_FILES)))

ISO_PATH := iso
ISO_FILE := bootable_kernel.iso
ISO      := $(ISO_PATH)/$(ISO_FILE)

GRUB_CFG := grub.cfg

ISO_SRC := $(ISO_PATH)/boot/$(NAME) $(ISO_PATH)/boot/grub/$(GRUB_CFG)

DOCKERFILE   := Dockerfile
DOCKER_IMAGE := kfs-builder
DOCKER_CMD   := grub-mkrescue -o $(ISO_FILE) $(ISO_PATH)

.PHONY: all clean fclean re run docker

all: $(ISO)

print:
	@echo "C source files: $(C_SRC)"
	@echo "Assembly source files: $(AS_SRC)"
	@echo "Object files: $(C_OBJS) $(AS_OBJS)"

$(NAME): $(C_OBJS) $(AS_OBJS)
	$(LD) $(LDFLAGS) -T $(LD_SCRIPT) -o $(NAME) $(C_OBJS) $(AS_OBJS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.asm
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -I$(INC_PATH) $< -o $@

$(ISO_SRC): $(NAME) $(GRUB_CFG)
	mkdir -p $(ISO_PATH)/boot/grub
	cp $(NAME) $(ISO_PATH)/boot/$(NAME)
	cp $(GRUB_CFG) $(ISO_PATH)/boot/grub/grub.cfg

$(ISO): $(ISO_SRC)
	docker build -t $(DOCKER_IMAGE) -f $(DOCKERFILE) .
	docker run --rm -it -v "$(CURDIR):/workspace" -w /workspace $(DOCKER_IMAGE) $(DOCKER_CMD)
	mv $(ISO_FILE) $(ISO)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

docker:

clean:
	rm -rf $(OBJ_PATH)

fclean: clean
	rm -rf $(NAME) $(ISO_PATH)

re: fclean all
