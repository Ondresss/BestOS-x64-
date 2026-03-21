CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -c
LDFLAGS = -m elf_i386 -Ttext 0x7E00 --oformat binary
ASFLAGS = -f bin
OBJFLAGS = -f elf32

KERNEL_OBJS = kernel.o functions.o vga.o io.o strings.o serial.o keyboard.o

run: disk.img

boot.bin: fe-boot.asm
	$(AS) $(ASFLAGS) fe-boot.asm -o boot.bin

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

serial.o: ./drivers/serial.o
	$(CC) $(CFLAGS) ./drivers/serial.c -o serial.o
keyboard.o: ./drivers/keyboard.o
	$(CC) $(CFLAGS) ./drivers/keyboard.c -o keyboard.o

strings.o: ./utils/strings.o
	$(CC) $(CFLAGS) ./utils/strings.c -o strings.o

io.o: ./arch/io.c
	$(CC) $(CFLAGS) ./arch/io.c -o io.o

vga.o: ./drivers/vga.c
	$(CC) $(CFLAGS) ./drivers/vga.c -o vga.o

functions.o: functions.asm
	$(AS) $(OBJFLAGS) functions.asm -o functions.o

kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o kernel.bin $(KERNEL_OBJS)

disk.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > disk.img

clean:
	rm -f *.bin *.o *.img ./arch/*.o ./drivers/*.o