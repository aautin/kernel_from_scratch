# Kernel From Scratch

A small 32-bit x86 kernel written in C and NASM assembly. It boots through GRUB, configures the processor and legacy interrupt controller, then presents a VGA text-mode shell that accepts keyboard input.

> This is an educational kernel, not a Linux distribution and not a drop-in replacement for the Linux kernel. The Linux comparison below is included to make that boundary explicit.

## Contents

- [Build and run](#build-and-run)
- [Using the kernel](#using-the-kernel)
- [How the kernel works](#how-the-kernel-works)
- [This project and Linux](#this-project-and-linux)
- [Project structure](#project-structure)
- [Current scope](#current-scope)

## Build and run

### Requirements

Install the following on the host:

- `make`
- GCC with 32-bit compilation support (`gcc -m32`)
- GNU `ld` with i386 output support
- `nasm`
- Docker, used to build the GRUB bootable ISO with `grub-mkrescue`
- `qemu-system-i386`, used by the `run` target

The Docker image installs the ISO-specific packages itself:

- `grub2-common`
- `grub-pc-bin`
- `xorriso`

### Commands

```sh
# Build kfs.elf, place it in the ISO tree, and create iso/bootable_kernel.iso
make

# Build and boot the ISO in an i386 QEMU machine
make run

# Remove intermediate objects
make clean

# Remove objects, the ELF, and the ISO directory contents
make fclean

# Clean and rebuild everything
make re
```

The build pipeline is:

```mermaid
flowchart LR
		C[C sources] --> CO[32-bit C objects]
		A[NASM sources] --> AO[ELF32 assembly objects]
		CO --> L[ld -m elf_i386]
		AO --> L
		L --> ELF[kfs.elf]
		ELF --> ISO[GRUB ISO via grub-mkrescue]
		CFG[grub.cfg] --> ISO
		ISO --> Q[QEMU i386]
```

The kernel is freestanding: it is compiled with `-ffreestanding`, `-fno-builtin` and `-nostdlib`, then linked with the project linker script rather than a host operating-system runtime.

## Using the kernel

After `make run`, GRUB loads `kfs.elf` and the screen shows the `42> ` prompt. Commands are typed through the PS/2 keyboard interrupt handler.

| Command | Purpose |
| --- | --- |
| `help` | List the available commands |
| `clear` | Clear the current terminal |
| `last` | Run the previous command again |
| `pmultiboot` | Print memory information supplied by GRUB |
| `pstack` | Dump the current kernel stack |
| `reboot` | Ask the PS/2 controller to reboot the machine |

Keyboard controls:

- `Caps Lock` toggles the simple alphabetic uppercase mode.
- `Backspace` deletes input characters.
- `Enter` executes the current input line.
- `Tab` switches between the two in-memory terminal screens.
- `Ctrl` cycles the terminal color scheme.
- `Up` and `Down` scroll the current terminal buffer.

## How the kernel works

### Boot sequence

The boot entry point is reached by GRUB in 32-bit protected mode. GRUB passes the Multiboot information pointer in `EBX`; the assembly entry point forwards it to `kernel_main`.

```mermaid
sequenceDiagram
		participant GRUB
		participant Entry as kernel_entry
		participant Main as kernel_main
		participant CPU
		participant PIC
		participant Keyboard

		GRUB->>Entry: Load kfs.elf and place mbi pointer in EBX
		Entry->>Main: push EBX, then call kernel_main
		Main->>Main: Initialize shell and terminal
		Main->>CPU: Load GDT and reload segment registers
		Main->>PIC: Remap 8259A vectors to 0x20 and 0x28
		Main->>CPU: Install keyboard gate at IDT vector 0x21
		Main->>CPU: load IDT, then enable interrupts with sti
		CPU->>Keyboard: Wait in hlt until an IRQ arrives
		Keyboard->>CPU: IRQ1 with a scan code
		CPU->>Main: irq1_stub -> keyboard handler -> EOI -> iretd
```

The linker script places the GDT section near `0x800`, loads the Multiboot header and kernel at the 1 MiB boundary, defines the BSS boundaries, and reserves a 16 KiB kernel stack. The boot code disables interrupts while this setup is performed.

### CPU protection and interrupts

The kernel creates a Global Descriptor Table (GDT) containing null, kernel code/data/stack, and user code/data/stack descriptors. The user descriptors are prepared for future work; this repository does not switch to user mode.

The Interrupt Descriptor Table (IDT) has 256 entries, but this version installs only the keyboard gate. The legacy 8259A Programmable Interrupt Controller is remapped so hardware IRQs do not overlap CPU exception vectors:

```text
CPU exceptions     0x00 - 0x1F
Master PIC IRQs    0x20 - 0x27   (IRQ1 keyboard = 0x21)
Slave PIC IRQs     0x28 - 0x2F
```

The IRQ1 assembly stub saves registers, reads port `0x60`, calls the C keyboard handler, sends an End Of Interrupt command to the master PIC, restores registers, and returns with `iretd`.

### Keyboard, terminal, and shell flow

```mermaid
flowchart TD
		K[PS/2 keyboard] -->|scan code on IRQ1| S[irq1_stub.asm]
		S --> H[keyboard_interrupt_handler]
		H -->|printable key| T[terminal input buffer]
		H -->|Enter| E[terminal_get_input]
		E --> X[shell_execute]
		X --> C[Command function]
		C --> P[printk / terminal output]
		T --> V[VGA text memory 0xB8000]
		P --> V
		H -->|Tab| D[Switch one of two screens]
		H -->|Ctrl| Y[Cycle color scheme]
```

The terminal keeps two complete screen buffers. Each buffer is 80 columns by 50 logical rows: 25 visible VGA rows plus 25 rows that can be scrolled into view. `put_screen` copies the selected buffer to VGA text memory, while the hardware cursor is updated through VGA ports `0x3D4` and `0x3D5`.

`printk` is a small project-local formatter. It supports the format cases used by the shell (`%c`, `%s`, `%d`/`%i`, `%x`, `%llx`, `%p`, and `%%`); it does not provide the complete hosted C `printf` contract.

## This project and Linux

The following table separates general operating-system concepts from behavior implemented here and behavior normally associated with official Linux.

| Topic | This kernel | Official Linux, in general |
| --- | --- | --- |
| Boot | GRUB Multiboot loads one ELF kernel and passes boot metadata. | Linux can be loaded by several boot protocols and bootloaders; the exact path depends on architecture and platform. |
| CPU mode | 32-bit x86 protected mode with a project-defined GDT. | Linux supports multiple architectures and uses architecture-specific setup; modern x86 systems commonly run a 64-bit kernel. |
| Memory | Reads selected Multiboot memory information and reserves static kernel data/stack. | Provides virtual memory, paging, allocators, protection, mappings, reclaim, and many memory-management subsystems. |
| Interrupts | Uses one IDT entry and the legacy 8259A PIC for keyboard IRQ1. | Uses architecture-specific interrupt facilities, commonly local and I/O APICs on x86, plus extensive exception and interrupt handling. |
| Input/output | Reads PS/2 port `0x60` and writes VGA text memory at `0xB8000`. | Uses a large driver model and supports many buses, input devices, display paths, and consoles. |
| User programs | None. The shell executes built-in kernel functions in ring 0. | User processes run with isolation, scheduling, system calls, signals, and process credentials. |
| Storage/filesystems | None. | Linux exposes storage through block drivers and the VFS/filesystem stack. |
| Shell | A fixed six-command shell compiled into the kernel. | A shell such as Bash is a user-space program; it is not the Linux kernel itself. |
| Scheduling | No scheduler; the kernel halts while idle and wakes for enabled interrupts. | Schedules runnable tasks across CPUs according to kernel scheduling policies. |

Concepts such as GDTs, IDTs, PICs, interrupt gates, privilege rings, port I/O, and `hlt` are real x86 mechanisms. Their small-scale use here is specific to this project. The project should therefore be read as a learning implementation of selected kernel building blocks, not as a miniature copy of Linux's complete architecture.

## Project structure

```text
.
├── Makefile                 Build graph, compiler flags, ISO and QEMU targets
├── README.md                This guide
├── inc/                     Public headers shared by C modules
│   ├── gdt.h                GDT entries, selectors, and descriptors
│   ├── idt.h                IDT entry format and keyboard vector
│   ├── keyboard.h           Keyboard handler declaration
│   ├── multiboot.h          Multiboot structures and memory-map types
│   ├── pic.h                8259A PIC ports, commands, and masks
│   ├── shell.h              Shell interface
│   ├── terminal.h           Terminal and input/output interface
│   ├── helper/              Freestanding string, memory, character, and print APIs
│   └── io/                  Port-I/O and VGA interfaces
├── src/
│   ├── kernel/
│   │   ├── boot/            C entry point and Multiboot reporting
│   │   ├── sections/        Multiboot header and GDT storage sections
│   │   ├── interrupt/       IDT/PIC setup and IRQ assembly/C handlers
│   │   ├── io/              x86 port access and VGA text output
│   │   ├── helper/          Freestanding utility implementations
│   │   ├── gdt.c            GDT construction and segment reload
│   │   ├── terminal.c       Two-screen terminal, scrolling, input, and output
│   │   └── shell.c          Built-in command table and command execution
│   └── misc/
│       ├── linker.ld        Kernel layout, entry point, BSS, and stack
│       ├── grub.cfg         GRUB menu entry using the Multiboot loader
│       └── Dockerfile       ISO builder image definition
├── iso/                     Generated bootable ISO tree (ignored by Git)
└── obj/                     Generated object files (ignored by Git)
```

The main ownership boundaries are:

- `boot` and `sections`: get from GRUB to C code and define link-time layout.
- `gdt`, `interrupt`, and `pic`: establish the processor and hardware interrupt environment.
- `io` and `terminal`: turn low-level device access into a text interface.
- `keyboard` and `shell`: turn scan codes into commands.
- `helper`: provide the small freestanding runtime this kernel needs instead of relying on libc.

## Current scope

Implemented:

- GRUB Multiboot booting on 32-bit x86.
- GDT setup and kernel segment reload.
- IDT registration for keyboard IRQ1.
- 8259A PIC remapping and keyboard interrupt acknowledgement.
- VGA text output, cursor control, scrolling, and two terminal screens.
- Basic keyboard input, backspace, caps mode, color switching, and shell commands.
- Multiboot memory information and kernel stack inspection commands.

Not implemented yet:

- Paging and a general physical/virtual memory allocator.
- User mode, processes, threads, scheduling, and system calls.
- CPU exception handlers beyond the default hardware behavior.
- Timers, disks, filesystems, networking, USB, and modern display output.
- A libc, complete `printf`, command parsing with arguments, or persistent storage.
