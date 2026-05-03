[bits 16]
[org 0x7c00]

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

boot_message:
    mov bp, msg
    mov ah, 0x13
    mov cx, 11
    mov al, 0x01
    mov dh, 0
    mov dl, 0
    mov bx, 0x0007
    int 0x10

load_sector:
    mov ax, 0x0000
    mov es, ax
    mov bx, 0x7e00

    mov ah, 0x02
    mov al, 100
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    int 0x13
    jc disk_error

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:init_pm

disk_error:
    jmp $

[bits 32]
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x90000
    mov ebp, esp

    jmp 0x7e00
    jmp $

msg db "Booting OS ", 0

align 16
gdt_start:
    dq 0x0
gdt_code:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xaa55