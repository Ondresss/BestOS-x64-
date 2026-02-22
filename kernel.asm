[bits 16] ; use 16-bit real mode
[org 0x7e00] ; starting address whene loaded by BIOS

kernel_message:
    mov bp,msg
    mov ah,0x13
    mov cx,13
    mov al,0x00
    mov dh,12
    mov dl,4
    mov bx,0x02
    int 0x10
    xor ax,ax
    cpuid
end:
    hlt ; halt CPU


msg db "Loading Kernel",0
times 510-($-$$) db 0; ; fills the gap with 0
dw 0xaa55