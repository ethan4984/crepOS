#include <int/apic.hpp>
#include <mm/vmm.hpp>
#include <acpi/madt.hpp>
#include <drivers/hpet.hpp>

namespace apic {

static lib::vector<ioapic> ioapic_list;

uint32_t ioapic::read(uint32_t reg) {
    *reinterpret_cast<volatile uint32_t*>(madt1_ptr.ioapic_addr + vmm::high_vma) = reg;
    return *reinterpret_cast<volatile uint32_t*>(madt1_ptr.ioapic_addr + 16 + vmm::high_vma);
}

void ioapic::write(uint32_t reg, uint32_t data) {
    *reinterpret_cast<volatile uint32_t*>(madt1_ptr.ioapic_addr + vmm::high_vma) = reg;
    *reinterpret_cast<volatile uint32_t*>(madt1_ptr.ioapic_addr + 16 + vmm::high_vma) = data;
}

size_t ioapic::read_redirection(uint32_t gsi) {
    uint32_t reg = ((gsi - madt1_ptr.gsi_base) * 2) + 16;
    return read(reg) | ((size_t)read(reg + 1) << 32);
}

void ioapic::write_redirection(uint32_t gsi, size_t data) {
    uint32_t reg = ((gsi - madt1_ptr.gsi_base) * 2) + 16;
    write(reg, (uint32_t)data);
    write(reg + 1, (uint32_t)(data >> 32));
}

void ioapic::mask_gsi(uint32_t gsi) {
    write_redirection(gsi, read_redirection(gsi) | (1 << 16));
}

void ioapic::unmask_gsi(uint32_t gsi) {
    write_redirection(gsi, read_redirection(gsi) & (1 << 16));
}

uint32_t ioapic::max_gsi() {
    uint32_t data = read(1) >> 16;
    return data & ~(1 << 7);
}

void ioapic::mask_all_gsi() {
    for(uint32_t i = madt1_ptr.gsi_base; i < max_gsi(); i++)
        mask_gsi(i);
}

void ioapic::redirect_gsi(uint8_t vector, uint32_t gsi, uint16_t flags) {
    size_t redirect = vector;

    if(flags & (1 << 1)) {
        redirect |= 1 << 13; 
    }

    if(flags & (1 << 3)) {
        redirect |= 1 << 15;
    }

    write_redirection(gsi, redirect);
}

ioapic ioapic_validate(uint32_t gsi) {
    for(size_t i = 0; i < ioapic_list.size(); i++) {
        if(ioapic_list[i].madt1_ptr.gsi_base <= gsi && ioapic_list[i].max_gsi() >= gsi) {
            return ioapic_list[i];
        }
    }
    return ioapic();
}

void timer_calibrate(uint64_t ms) {
    lapic->write(lapic->timer_divide_conf(), 0x3);
    lapic->write(lapic->timer_inital_count(), ~0); 

    ksleep(ms);

    uint32_t ticks = ~0 - lapic->read(lapic->timer_current_count());

    lapic->write(lapic->timer_lvt(), 32 | 0x20000);
    lapic->write(lapic->timer_divide_conf(), 0x3);
    lapic->write(lapic->timer_inital_count(), ticks); 
}

x2apic::x2apic() {
    cpuid_state cpu_state = cpuid(1, 0);

    if(!(cpu_state.rcx & (1 << 21)))
        return;

    size_t apic_base = rdmsr(msr_lapic_base);
    apic_base |= 0b11 << 10;
    wrmsr(msr_lapic_base, apic_base);
}

void init() {
    madt_ptr = find_SDT<madt>("APIC");

    for(size_t i = 0; i < madt_ptr->acpihdr_ptr.length - (sizeof(madt_ptr->acpihdr_ptr) + sizeof(madt_ptr->lapic_addr) + sizeof(madt_ptr->flags)); i++) {
        uint8_t entry_type = madt_ptr->entries[i++];
        uint8_t entry_size = madt_ptr->entries[i++];

        switch(entry_type) {
            case 0:
                madt0_list.push(*reinterpret_cast<madt0*>(&madt_ptr->entries[i]));
                break;
            case 1:
                madt1_list.push(*reinterpret_cast<madt1*>(&madt_ptr->entries[i]));
                ioapic_list.push(ioapic(*reinterpret_cast<madt1*>(&madt_ptr->entries[i])));
                break;
            case 2:
                madt2_list.push(*reinterpret_cast<madt2*>(&madt_ptr->entries[i]));
                break;
            case 4:
                madt4_list.push(*reinterpret_cast<madt4*>(&madt_ptr->entries[i]));
                break;
            case 5:
                madt5_list.push(*reinterpret_cast<madt5*>(&madt_ptr->entries[i]));
        }
        i += entry_size - 3;
    }

    print("[ACPI] core count detected {}\n", madt0_list.size());

    cpuid_state cpu_state = cpuid(1, 0);

    if(cpu_state.rcx & (1 << 21)) {
        lapic = new x2apic;
    } else {
        lapic = new xapic;
    }

    outb(0x20, 0x11);
    outb(0xa0, 0x11);
    outb(0x21, 0x20);
    outb(0xa1, 0x28);
    outb(0x21, 0x4);
    outb(0xa1, 0x2);
    outb(0x21, 0x1);
    outb(0xa1, 0x1);
    outb(0x21, 0x0);
    outb(0xa1, 0x0);

    outb(0xa1, 0xff);
    outb(0x21, 0xff);

    for(size_t i = 0; i < ioapic_list.size(); i++) {
        ioapic_list[i].mask_all_gsi();
    }

    uint8_t mapped_irqs[16] = { 0 };
    for(size_t i = 0; i < madt2_list.size(); i++) {
        madt2 iso = madt2_list[i];
        ioapic_validate(iso.gsi).redirect_gsi(iso.irq_src + 32, iso.gsi, iso.flags);
        if(iso.gsi < 16)
            mapped_irqs[iso.gsi] = 1;
    }

    for(size_t i = 0; i < 16; i++) {
        if(!mapped_irqs[i])
            ioapic_validate(i).redirect_gsi(i + 32, i, 0);
    }

    ioapic_validate(2).mask_gsi(2);

    lapic->write(lapic->sint(), lapic->read(lapic->sint()) | 0x1ff); // enable spurious interrupts

    asm volatile ("mov %0, %%cr8" :: "r"(0ull));
}

}
