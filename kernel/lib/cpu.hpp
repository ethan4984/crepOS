#ifndef CPU_HPP_
#define CPU_HPP_

#include <stddef.h>
#include <stdint.h>

constexpr size_t msr_efer = 0xc0000080;
constexpr size_t msr_star = 0xc0000081;
constexpr size_t msr_lstar = 0xc0000082;
constexpr size_t msr_cstar = 0xc0000083;
constexpr size_t msr_sfmask = 0xc0000084;

constexpr size_t msr_fs_base = 0xc0000100;
constexpr size_t msr_gs_base = 0xc0000101;
constexpr size_t kernel_gs_base = 0xc0000102;

constexpr size_t com1 = 0x3f8;
constexpr size_t com2 = 0x2f8;
constexpr size_t com3 = 0x3e8;
constexpr size_t com4 = 0x2e8;

struct regs {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t isr_number;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs; 
    uint64_t rflags; 
    uint64_t rsp;
    uint64_t ss;
};

struct cpuid_state {
    uint64_t leaf;
    uint64_t subleaf;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
};

inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

inline void outw(uint16_t port, uint16_t data) {
    asm volatile("outw %0, %1" :: "a"(data), "Nd"(port));
}

inline void outd(uint16_t port, uint32_t data) {
    asm volatile("outl %0, %1" :: "a"(data), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0" : "=a"(data):"Nd"(port));
    return data;
}

inline uint16_t inw(uint16_t port) {
    uint16_t data;
    asm volatile("inw %1, %0" : "=a"(data):"Nd"(port));
    return data;
}

inline uint32_t ind(uint16_t port) {
    uint32_t data;
    asm volatile("inl %1, %0" : "=a"(data):"Nd"(port));
    return data;
}

inline uint64_t rdmsr(uint32_t msr) {
    uint64_t rax, rdx;
    asm volatile ("rdmsr" : "=a"(rax), "=d"(rdx) : "c"(msr) : "memory");
    return (rdx << 32) | rax;
}

inline void wrmsr(uint32_t msr, uint64_t data) {
    uint64_t rax = (uint32_t)data;
    uint64_t rdx = data >> 32;
    asm volatile ("wrmsr" :: "a"(rax), "d"(rdx), "c"(msr));
}

inline cpuid_state cpuid(size_t leaf, size_t subleaf) {
    cpuid_state ret = { leaf, subleaf, 0, 0, 0, 0 };

    size_t max;
    asm volatile ("cpuid" : "=a"(max) : "a"(leaf & 0x80000000) : "rbx", "rcx", "rdx");

    if(leaf > max) {
        return ret;
    }

    asm volatile ("cpuid" : "=a"(ret.rax), "=b"(ret.rbx), "=c"(ret.rcx), "=d"(ret.rbx) : "a"(leaf), "c"(subleaf));

    return ret;
}

inline void swapgs(void) {
    asm volatile ("swapgs" ::: "memory");
}

inline void set_kernel_gs(uintptr_t addr) {
    wrmsr(msr_gs_base, addr);
}

inline uint64_t get_kernel_gs() {
    return rdmsr(msr_gs_base);
}

inline void set_user_gs(uintptr_t addr) {
    wrmsr(kernel_gs_base, addr);
}

inline void set_user_fs(uintptr_t addr) {
    wrmsr(msr_fs_base, addr);
}

inline uintptr_t get_user_gs(void) {
    return rdmsr(kernel_gs_base);
}

inline uintptr_t get_user_fs(void) {
    return rdmsr(msr_fs_base);
}

template <typename T>
void spin_lock(T *lock) {
    while(__atomic_test_and_set(lock, __ATOMIC_ACQUIRE));
}

template <typename T>
void spin_release(T *lock) {
    __atomic_clear(lock, __ATOMIC_RELEASE);
}

inline const char *exception_messages[] = { "Divide by zero",
                                            "Debug",
                                            "NMI",
                                            "Breakpoint",
                                            "Overflow",
                                            "Bound Range Exceeded",
                                            "Invaild Opcode",
                                            "Device Not Available", 
                                            "Double fault", 
                                            "Co-processor Segment Overrun",
                                            "Invaild TSS",
                                            "Segment not present",
                                            "Stack-Segment Fault",
                                            "GPF",
                                            "Page Fault",
                                            "Reserved",
                                            "x87 Floating Point Exception",
                                            "allignement check",
                                            "Machine check",
                                            "SIMD floating-point exception",
                                            "Virtualization Excpetion",
                                            "Deadlock",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Reserved",
                                            "Security Exception",
                                            "Reserved",
                                            "Triple Fault",
                                            "FPU error"
                                         };

#endif
