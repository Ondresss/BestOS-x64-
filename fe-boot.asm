[bits 16] ; use 16-bit real mode
[org 0x7c00] ; starting address whene loaded by BIOS

boot_message:
    mov bp,msg
    mov ah,0x13
    mov cx,10
    mov al,0x00
    mov dh,7
    mov dl,4
    mov bx,0x02
    int 0x10

load_sector:
    xor ax,ax
    mov es,ax ; ES cannot be directly assigned
    mov bx,0x7e00 ; // memory where sectors will be placed
    mov al,10 ; load 10 sectors
    mov ch,0 ; // cylinder
    mov cl,0x02 ; // CC SSSSSS sector = 2
    mov dl,0x80  ; drive 0 (default)
    mov dh,0 ; // CHS -  HEAD 0

    mov ah,0x02 ; read desired sectors into memory
    int 0x13

end:
    jmp 0x0000:0x7e00


msg db "Booting OS",0
times 510-($-$$) db 0; ; fills the gap with 0
dw 0xaa55