DOCKER_IMAGE = kfs-build:latest
DOCKER_WORKDIR = /kfs-build
DOCKER_RUN = docker run --rm -v $(PWD):$(DOCKER_WORKDIR) -w $(DOCKER_WORKDIR) $(DOCKER_IMAGE)

SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build

SRCS_C = kernel.c lib.c tty.c keyboard.c io.c
SRCS_S = boot.s
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS_C:.c=.o) $(SRCS_S:.s=.o))

GCC = gcc
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -nostdlib -nodefaultlibs
INCLUDE_DIRS = -Iinclude

all : $(OBJ_DIR) $(BUILD_DIR) $(BUILD_DIR)/kfs run

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilation rules for C and assembly files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(GCC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	nasm -f elf32 $< -o $@

# Generation of the final kernel binary
$(BUILD_DIR)/kfs: $(OBJS)
	ld -m elf_i386 -T linker.ld -o $@ $(OBJS)

kfs.iso: docker-build $(BUILD_DIR)/kfs
	mkdir -p iso/boot/grub
	cp $(BUILD_DIR)/kfs iso/boot/kfs
	cp boot/grub/grub.cfg iso/boot/grub/grub.cfg
	$(DOCKER_RUN) grub-mkrescue -o kfs.iso iso --modules="normal multiboot" --locales="" --fonts=""
	#replace by grub2-mkrescue for fedora

run: kfs.iso
	qemu-system-i386 -cdrom kfs.iso

docker-build:
	docker build -t $(DOCKER_IMAGE) -f Dockerfile .

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR) iso
	rm -f kfs.iso

.PHONY: all clean docker-build run

