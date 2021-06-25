#include <int/gdt.hpp>

namespace x86 {

template <typename T>
void gdt::append(T &desc) {
    gdt_reg.limit += sizeof(desc.raw);

    if(!gdt_reg.offset) {
        gdt_reg.offset = (size_t)(new uint8_t[gdt_reg.limit + 1]);
    } else {
        gdt_reg.offset = (size_t)kmm::realloc((void*)gdt_reg.offset, gdt_reg.limit + 1);
    }

    [this]<typename F>(F &raw) {
        *reinterpret_cast<F*>(gdt_reg.offset + gdt_reg.limit - sizeof(raw) + 1) = raw;
    } (desc.raw);

    desc.selector = gdt_reg.limit - sizeof(desc.raw) + 1;
}

segment_descriptor::segment_descriptor(uint8_t a, uint8_t g) {
    raw.limit = 0;
    raw.base_low = 0;
    raw.base_mid = 0;
    raw.access = a;
    raw.granularity = g;
    raw.base_high = 0;
}

void gdt_init() {
    segment_descriptor null(0, 0);
    segment_descriptor kernel_code64(0b10011010, 0b00100000);
    segment_descriptor kernel_data64(0b10010110, 0);
    segment_descriptor user_data64(0b11110010, 0);
    segment_descriptor user_code64(0b11111010, 0b00100000);

    system_gdt.append(null);
    system_gdt.append(kernel_code64);
    system_gdt.append(kernel_data64);
    system_gdt.append(user_data64);
    system_gdt.append(user_code64);

    lgdt((size_t)&system_gdt.gdt_reg);
}

tss_descriptor::tss_descriptor(uint64_t addr) {
    raw.length = 104;
    raw.base_low = addr & 0xffff;
    raw.base_mid = addr >> 16 & 0xff;
    raw.flags1 = 0b10001001;
    raw.flags2 = 0;
    raw.base_high = addr >> 24 & 0xff;
    raw.base_high32 = addr >> 32 & 0xffffffff;
    raw.reserved = 0;
}

tss::tss() {
    rsp0 = pmm::alloc(2) + 0x2000 + vmm::high_vma;
    rsp1 = pmm::alloc(2) + 0x2000 + vmm::high_vma;
    rsp2 = pmm::alloc(2) + 0x2000 + vmm::high_vma;

    tss_descriptor desc((size_t)this);
    system_gdt.append(desc);

    lgdt((size_t)&system_gdt.gdt_reg);
    ltr(desc.selector); 
}

}
