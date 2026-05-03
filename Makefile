CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -c
LDFLAGS = -m elf_i386 -Ttext 0x7E00 --oformat binary
ASFLAGS = -f bin
OBJFLAGS = -f elf32

KERNEL_OBJS = kernel_entry.o kernel.o functions.o vga.o io.o strings.o serial.o keyboard.o ide.o cli.o commands.o diskAccess.o diskAccessUtils.o idt.o isr.o pic.o timer.o scheduler.o

kernel_entry.o: kernel_entry.asm
	$(AS) $(OBJFLAGS) kernel_entry.asm -o kernel_entry.o

diskAccess.o: ./diskAccess/diskAccess.o
	$(CC) $(CFLAGS) ./diskAccess/diskAccess.c -o diskAccess.o
diskAccessUtils.o: ./diskAccess/diskAccessUtils.o
	$(CC) $(CFLAGS) ./diskAccess/diskAccessUtils.c -o diskAccessUtils.o
boot.bin: fe-boot.asm
	$(AS) $(ASFLAGS) fe-boot.asm -o boot.bin

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@


scheduler.o: ./scheduler/scheduler.o
	$(CC) $(CFLAGS) ./scheduler/scheduler.c -o scheduler.o

serial.o: ./drivers/serial.o
	$(CC) $(CFLAGS) ./drivers/serial.c -o serial.o
ide.o: ./drivers/ide.o
	$(CC) $(CFLAGS) ./drivers/ide.c -o ide.o

cli.o: ./cli/cli.o
	$(CC) $(CFLAGS) ./cli/cli.c -o cli.o
commands.o: ./cli/commands.o
	$(CC) $(CFLAGS) ./cli/commands.c -o commands.o

keyboard.o: ./drivers/keyboard.o
	$(CC) $(CFLAGS) ./drivers/keyboard.c -o keyboard.o

strings.o: ./utils/strings.o
	$(CC) $(CFLAGS) ./utils/strings.c -o strings.o

io.o: ./arch/io.c
	$(CC) $(CFLAGS) ./arch/io.c -o io.o

timer.o: ./arch/timer.c
	$(CC) $(CFLAGS) ./arch/timer.c -o timer.o

idt.o: ./arch/idt.c
	$(CC) $(CFLAGS) ./arch/idt.c -o idt.o

isr.o: ./arch/isr.c
	$(CC) $(CFLAGS) ./arch/isr.c -o isr.o

pic.o: ./arch/pic.c
	$(CC) $(CFLAGS) ./arch/pic.c -o pic.o

vga.o: ./drivers/vga.c
	$(CC) $(CFLAGS) ./drivers/vga.c -o vga.o

functions.o: functions.asm
	$(AS) $(OBJFLAGS) functions.asm -o functions.o

kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o kernel.bin $(KERNEL_OBJS)

disk.img: boot.bin kernel.bin
	cp sd.img disk.img
	dd if=boot.bin of=disk.img bs=1 count=446 conv=notrunc
	dd if=kernel.bin of=disk.img bs=512 seek=1 conv=notrunc
	#mcopy -o -i disk.img@@1048576 ./test.bin ::/TEST.BIN

run: disk.img
	qemu-system-x86_64 -drive file=disk.img,format=raw,index=0,media=disk -serial stdio

clean:
	rm -f *.bin *.o ./arch/*.o ./drivers/*.o ./cli/*.o ./diskAccess/*.o ./scheduler/*.o