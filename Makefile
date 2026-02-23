
CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -c
LDFLAGS = -m elf_i386 -Ttext 0x7E00 --oformat binary
ASFLAGS = -f bin
OBJFLAGS = -f elf32

run: disk.img

boot.bin: fe-boot.asm
	$(AS) $(ASFLAGS) fe-boot.asm -o boot.bin

functions.o: functions.asm
	$(AS) $(OBJFLAGS) functions.asm -o functions.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -o kernel.o

kernel.bin: kernel.o functions.o
	$(LD) $(LDFLAGS) -o kernel.bin kernel.o functions.o

disk.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > disk.img

clean:
	rm -f *.bin *.o *.img