[bits 32]
section .text
    global getCPUVendor
    global common_isr_handler

getCPUVendor:
    enter 0,0
    push ebx
    push edi
    mov eax,0
    cpuid
    mov edi,dword [ebp + 8]
    mov dword [edi],ebx
    mov dword [edi + 4],edx
    mov dword [edi + 8],ecx
    pop edi
    pop ebx
    leave
    ret

[extern generic_isr_handler]

common_isr_handler:
    pusha

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push eax
    call generic_isr_handler
    mov esp, eax

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret

global isr13
isr13:
    cli
    mov al, '!'
    mov [0xb8000], al
    jmp $

global isr32
isr32:
    push dword 0
    push dword 32
    jmp common_isr_handler

global isr33
isr33:
    push dword 0
    push dword 33
    jmp common_isr_handler

global isr46
isr46:
    push dword 0
    push dword 46
    jmp common_isr_handler

global isr47
isr47:
    push dword 0
    push dword 47
    jmp common_isr_handler

global isr48
isr48:
    push dword 0
    push dword 48
    jmp common_isr_handler