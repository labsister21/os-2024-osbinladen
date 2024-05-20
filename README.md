# os-2024-osbinladen
![](other/Screenshot.png)

## Requirements
- Linux Environment
- [Netwide Assembler](https://www.nasm.us/) 
- [GNU C Compiler](https://man7.org/linux/man-pages/man1/gcc.1.html) 
- [GNU Linker](https://linux.die.net/man/1/ld)
- [QEMU - System i386](https://www.qemu.org/docs/master/system/target-i386.html) 
- [GNU Make](https://www.gnu.org/software/make/)
- [genisoimage](https://linux.die.net/man/1/genisoimage)

## How To Run

- Install Requirements
```
sudo apt update
sudo apt install -y nasm gcc qemu-system-x86 make genisoimage
```
- Clone and open the repository folder
```
git clone https://github.com/labsister21/os-2024-osbinladen.git
cd os-2024-osbinladen
```
- Run the program
```
make lock1
```

## Directory tree
```
.
├── bin
│   ├── OS2024.iso
│   ├── asmGaming.o
│   ├── charframe.o
│   ├── clock
│   ├── cmos.o
│   ├── context_switch.o
│   ├── decoyy.tx
│   ├── disk.o
│   ├── fat32.o
│   ├── framebuffer.o
│   ├── gdt.o
│   ├── graphics.o
│   ├── idt.o
│   ├── inserter
│   ├── interrupt.o
│   ├── intsetup.o
│   ├── kernel
│   ├── kernel-entrypoint.o
│   ├── kernel.o
│   ├── keyboard.o
│   ├── paging.o
│   ├── peda-session-1.txt
│   ├── peda-session-unknown.txt
│   ├── portio.o
│   ├── process.o
│   ├── realModeGaming.o
│   ├── scheduler.o
│   ├── shell
│   ├── shell_elf
│   ├── storage.bin
│   ├── string.o
│   └── timer.o
├── other
│   ├── Screenshot.png
│   └── grub1
├── src
│   ├── archive_src
│   │   ├── header
│   │   │   └── framebuffer.h
│   │   └── framebuffer.c
│   ├── external
│   │   └── external-inserter.c
│   ├── font
│   │   └── basicFont.h
│   ├── header
│   │   ├── cpu
│   │   │   ├── gdt.h
│   │   │   ├── interrupt.h
│   │   │   └── portio.h
│   │   ├── driver
│   │   │   ├── charframe.h
│   │   │   ├── cmos.h
│   │   │   ├── disk.h
│   │   │   ├── graphics.h
│   │   │   ├── keyboard.h
│   │   │   └── timer.h
│   │   ├── filesystem
│   │   │   └── fat32.h
│   │   ├── interrupt
│   │   │   └── idt.h
│   │   ├── memory
│   │   │   └── paging.h
│   │   ├── process
│   │   │   └── process.h
│   │   ├── scheduler
│   │   │   └── scheduler.h
│   │   ├── stdlib
│   │   │   └── string.h
│   │   ├── user
│   │   │   ├── clock
│   │   │   │   └── user-clock.h
│   │   │   ├── shell_cmd.h
│   │   │   ├── user-buffer.h
│   │   │   ├── user-shell.h
│   │   │   └── user-syscall.h
│   │   ├── kernel-entrypoint.h
│   │   └── realModeGaming.h
│   ├── imgdata
│   │   └── attack.h
│   ├── stdlib
│   │   └── string.c
│   ├── asmGaming.s
│   ├── charframe.c
│   ├── cmos.c
│   ├── context_switch.s
│   ├── crt0.s
│   ├── disk.c
│   ├── fat32.c
│   ├── gdt.c
│   ├── graphics.c
│   ├── idt.c
│   ├── interrupt.c
│   ├── intsetup.s
│   ├── kernel-entrypoint.s
│   ├── kernel.c
│   ├── keyboard.c
│   ├── linker.ld
│   ├── menu.lst
│   ├── paging.c
│   ├── portio.c
│   ├── process.c
│   ├── realModeGaming.c
│   ├── scheduler.c
│   ├── shell_cmd.c
│   ├── timer.c
│   ├── user-buffer.c
│   ├── user-clock.c
│   ├── user-linker.ld
│   ├── user-shell.c
│   └── user-syscall.c
├── README.md
└── makefile
```

## Contributors

|Nama|NIM|Kelas|
|---|---|---|
|Abdul Rafi Radityo Hutomo|13522089|K01|
|Rayhan Fadhlan Azka|13522095|K01|
|Rayendra Althaf Taraka Noor|13522107|K01|
|William Glory Henderson|13522113|K01|
