[bits 16]
[org 0x7c00]

boot_message:
    mov bp, msg
    mov ah, 0x13
    mov cx, 10
    mov al, 0x00
    mov dh, 9
    mov dl, 4
    mov bx, 0x02
    int 0x10

load_sector:
    xor ax, ax
    mov es, ax
    mov bx, 0x7e00
    mov al, 10
    mov ch, 0
    mov cl, 0x02
    mov dl, 0x80
    mov dh, 0
    mov ah, 0x02
    int 0x13

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:init_pm

[bits 32]
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov esp, 0x90000

    call 0x7e00
    jmp $

msg db "Booting OS",0

gdt_start:
    dq 0x0
gdt_code:
    dw 0xffff, 0x0, 0x9a00, 0x00cf
gdt_data:
    dw 0xffff, 0x0, 0x9200, 0x00cf
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xaa55