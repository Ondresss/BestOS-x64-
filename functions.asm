[bits 32]
section .text:
    global getCPUVendor
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