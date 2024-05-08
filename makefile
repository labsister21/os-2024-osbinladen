UNAME := $(shell uname -s)

# Compiler & linker
ASM           = nasm

ifeq ($(UNAME), Linux)
	LIN           = ld
	CC            = gcc
else
	LIN           = x86_64-elf-ld
	CC            = x86_64-elf-gcc 
endif

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ifeq ($(UNAME), Linux)
	ISO_NAME      = OS2024
else
	ISO_NAME = mkisofs
endif

# Flags
WARNING_CFLAG = -Wall -Wextra
DEBUG_CFLAG   = -fshort-wchar -g

ifeq ($(UNAME), Linux)
	STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -fno-pie -nodefaultlibs -ffreestanding
else
	STRIP_CFLAG   = -nostdlib -fno-stack-protector -nodefaultlibs -ffreestanding -mno-sse -mno-avx
endif

CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386

DISK_NAME = storage

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

debug: all
	@qemu-system-i386 -s -S -drive file=bin/storage.bin,format=raw,if=ide,index=0,media=disk -cdrom bin/$(ISO_NAME).iso
run: all
	@qemu-system-i386 -s -drive file=bin/storage.bin,format=raw,if=ide,index=0,media=disk -cdrom bin/$(ISO_NAME).iso
all: build
build: iso
clean:
	rm -rf $(OUTPUT_FOLDER)/*.o $(OUTPUT_FOLDER)/*.iso $(OUTPUT_FOLDER)/kernel

kernel:
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/kernel.c -o $(OUTPUT_FOLDER)/kernel.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/gdt.c -o $(OUTPUT_FOLDER)/gdt.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/stdlib/string.c -o $(OUTPUT_FOLDER)/string.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/portio.c -o $(OUTPUT_FOLDER)/portio.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt.c -o $(OUTPUT_FOLDER)/interrupt.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/idt.c -o $(OUTPUT_FOLDER)/idt.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/keyboard.c -o $(OUTPUT_FOLDER)/keyboard.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/disk.c -o $(OUTPUT_FOLDER)/disk.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/realModeGaming.c -o $(OUTPUT_FOLDER)/realModeGaming.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/graphics.c -o $(OUTPUT_FOLDER)/graphics.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/paging.c -o $(OUTPUT_FOLDER)/paging.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/fat32.c -o $(OUTPUT_FOLDER)/fat32.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/charframe.c -o $(OUTPUT_FOLDER)/charframe.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/paging.c -o $(OUTPUT_FOLDER)/paging.o
	$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/intsetup.s -o $(OUTPUT_FOLDER)/intsetup.o
	$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/asmGaming.s -o $(OUTPUT_FOLDER)/asmGaming.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/kernel-entrypoint.s -o $(OUTPUT_FOLDER)/kernel-entrypoint.o
	@$(LIN) $(LFLAGS) bin/*.o -o $(OUTPUT_FOLDER)/kernel
	@echo Linking object files and generate elf32...
	@rm -f *.o


iso: kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1 $(OUTPUT_FOLDER)/iso/boot/grub/ 
	@cp $(SOURCE_FOLDER)/menu.lst $(OUTPUT_FOLDER)/iso/boot/grub/
	@genisoimage -R -b boot/grub/grub1 -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o $(OUTPUT_FOLDER)/OS2024.iso $(OUTPUT_FOLDER)/iso 
	@rm -r $(OUTPUT_FOLDER)/iso/

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g -I$(SOURCE_FOLDER) \
		$(SOURCE_FOLDER)/stdlib/string.c \
		$(SOURCE_FOLDER)/fat32.c \
		$(SOURCE_FOLDER)/external/external-inserter.c \
		-o $(OUTPUT_FOLDER)/inserter

user-shell:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/crt0.s -o crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user-shell.c -o user-shell.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/stdlib/string.c -o string.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/user-buffer.c -o user-buffer.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/shell_cmd.c -o shell_cmd.o
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o user-shell.o string.o user-buffer.o shell_cmd.o -o $(OUTPUT_FOLDER)/shell
	@echo Linking object shell object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		crt0.o user-shell.o string.o user-buffer.o shell_cmd.o -o $(OUTPUT_FOLDER)/shell_elf
	@echo Linking object shell object files and generate ELF32 for debugging...
	@size --target=binary $(OUTPUT_FOLDER)/shell
	@rm -f *.o

insert-shell: inserter user-shell
	@echo Inserting shell into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter shell 2 $(DISK_NAME).bin