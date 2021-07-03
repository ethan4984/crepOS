%macro popall 0

pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rsi
pop rdi
pop rbp
pop rdx
pop rcx
pop rbx
pop rax

%endmacro

global user_test

user_test:
    cli

    mov rax, rsp
    
    push 0x1b
    push rax
    pushfq
    push 0x23
    push .test
    iretq
.test:
    jmp $

global switch_task

switch_task:
    cli

    mov rsp, rdi
    popall
    add rsp, 16

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rdi

    jmp $

    iretq
