# IF2230-2024-OSbinLaden
![](other/Screenshot.png)

## **Table of Contents**
* [Authors](#authors)
* [Program Description](#program-description)
* [Required Program](#required-program)
* [How to Run The Program](#how-to-run-the-program)
* [Folders and Files Directory](#folders-and-files-description)

## **Authors** 
|Nama|NIM|Kelas|
|---|---|---|
|Abdul Rafi Radityo Hutomo|13522089|K01|
|Rayhan Fadhlan Azka|13522095|K01|
|Rayendra Althaf Taraka Noor|13522107|K01|
|William Glory Henderson|13522113|K01|

## **Program Description**
This program will serve as an introduction to kernel development and concretely demonstrate the subsystems present in an operating system. <br> 
The target platform for the operating system is x86 32-bit Protected Mode, which will be run using QEMU. <br>
The operating system includes Toolchain, Kernel, GDT, Interrupts, Drivers, File System, Paging, User Mode, Shell, Processes, Scheduler, Multitasking.

## **Required Program**
- [Linux Environment](https://learn.microsoft.com/en-us/windows/wsl/install)
- [Netwide Assembler](https://www.nasm.us/) 
- [GNU C Compiler](https://man7.org/linux/man-pages/man1/gcc.1.html) 
- [GNU Linker](https://linux.die.net/man/1/ld)
- [QEMU - System i386](https://www.qemu.org/docs/master/system/target-i386.html) 
- [GNU Make](https://www.gnu.org/software/make/)
- [genisoimage](https://linux.die.net/man/1/genisoimage)

## **How to Run The Program**

- Install Requirements
```
sudo apt install -y nasm gcc qemu-system-x86 make genisoimage
```
- Clone and open the repository folder
```
git clone https://github.com/labsister21/os-2024-osbinladen.git
```
- Change the current directory into the cloned repository 
```
cd os-2024-osbinladen
```
- Run the program
For the first time
```
make lock1
```
For the next use
```
make run
```

## **Folders and Files Directory**
```
.
├── bin
├── other
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