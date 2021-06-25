section .stivalehdr

dq stack
dw (1 << 0) | (1 << 1)
dw 0
dw 0
dw 0
dq 0

section .bss

align 0x10

resb 0x10000

stack:
