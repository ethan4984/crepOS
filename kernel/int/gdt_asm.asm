global lgdt

lgdt:
    lgdt [rdi]

    push 0x8
    push entry
    retfq
entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov ss, ax
    mov fs, ax 
    mov gs, ax

    ret

global ltr

ltr:
    ltr di
    ret
