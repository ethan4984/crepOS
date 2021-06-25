section .init

global _init:function

_init:
    push rbp
    mov rbp, rsp

section .fini

global _fini:function

_fini:
    push rbp
    mov rbp, rsp
