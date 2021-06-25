#ifndef GDT_HPP_
#define GDT_HPP_

#include <vector.hpp>

namespace x86 {

extern "C" void lgdt(size_t addr);
extern "C" void ltr(size_t selector);

constexpr size_t kernel64_cs = 0x8;
constexpr size_t kernel64_ds = 0x10;
constexpr size_t user64_cs = 0x23;
constexpr size_t user64_ds = 0x1b;

struct segment_descriptor {
    segment_descriptor(uint8_t access, uint8_t gran);
    segment_descriptor() = default;

    struct [[gnu::packed]] {
        uint16_t limit;
        uint16_t base_low;
        uint8_t base_mid;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
    } raw; 

    uint16_t selector;
};

struct tss_descriptor {
    tss_descriptor(uint64_t addr);
    tss_descriptor() = default;

    struct [[gnu::packed]] {
        uint16_t length;
        uint16_t base_low;
        uint8_t base_mid;
        uint8_t flags1;
        uint8_t flags2; 
        uint8_t base_high;
        uint32_t base_high32;
        uint32_t reserved;
    } raw;

    uint16_t selector;
};

struct [[gnu::packed]] tss {
    tss();

    uint32_t reserved;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint32_t reserved1;
    uint32_t reserved2;
    uint64_t ist1;
    uint64_t ist2; 
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t IOPB;
};

struct [[gnu::packed]] gdtr {
    gdtr() : limit(-1), offset(0) { }

    uint16_t limit;
    uint64_t offset;
};

struct gdt {
    template <typename T>
    void append(T &desc);

    gdtr gdt_reg;
};

inline gdt system_gdt;
void gdt_init();

}

#endif
