SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build

GCC = gcc
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -nostdlib -nodefaultlibs



all : $(OBJ_DIR) $(BUILD_DIR) $(BUILD_DIR)/kernel.bin

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilation rules for C and assembly files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(GCC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	nasm -f elf32 $< -o $@

# Generation of the final kernel binary
$(BUILD_DIR)/kernel.bin: $(OBJ_DIR)/boot.o $(OBJ_DIR)/kernel.o
	ld -m elf_i386 -T linker.ld -o $@ $(OBJ_DIR)/boot.o $(OBJ_DIR)/kernel.o

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

.PHONY: all clean