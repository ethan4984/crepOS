%macro pushall 0

push rax
push rbx
push rcx
push rdx
push rbp
push rdi
push rsi
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15

%endmacro

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

syscall_list:

.end:

syscall_cnt equ ((syscall_list.end - syscall_list) / 8)

global syscall_main

syscall_main:
    swapgs

    mov qword [gs:16], rsp ; save user stack
    mov rsp, qword [gs:8] ; init kernel stack

    sti

    push rcx ; rip
    push r11 ; rflags

    push 0x1b ; ss
    push qword [gs:16] ; rsp
    push r11 ; rflags
    push 0x23 ; cs
    push rcx ; rip

    push 0
    push 0

    pushall

    cmp rax, syscall_cnt
    jae .leave

    mov rdi, rsp
    call [syscall_list + rax * 8]

.leave:
    popall

    add rsp, 56

    pop r11 ; rflags
    pop rcx ; rip

    cli
   
    mov rdx, qword [gs:24] ; errno
    mov rsp, qword [gs:16] ; user stack

    swapgs

    o64 sysret ; ensure rex.w=1
