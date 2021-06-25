#include <sched/smp.hpp>
#include <acpi/madt.hpp>
#include <int/idt.hpp>
#include <int/apic.hpp>
#include <cpu.hpp>
#include <drivers/hpet.hpp>

extern symbol smp_core_init_begin;
extern symbol smp_core_init_end;

namespace smp {

static void prep_core(size_t stack, uint64_t pml4, uint64_t entry, uint64_t idt, uint64_t gdt, uint64_t core_index) { 
    uint64_t *parms = reinterpret_cast<uint64_t*>(0x500 + vmm::high_vma);
    parms[0] = stack;
    parms[1] = pml4;
    parms[2] = entry;
    parms[3] = idt;
    parms[4] = gdt;
    parms[5] = core_index;
}

static void core_bootstrap(size_t core_index) {
    new x86::tss;

    wrmsr(msr_gs_base, reinterpret_cast<size_t>(&cpus.data()[core_index]));

    apic::x2apic();
    apic::timer_calibrate(100);
    apic::lapic->write(apic::lapic->sint(), apic::lapic->read(apic::lapic->sint()) | 0x1ff);
    asm volatile ("mov %0, %%cr8\nsti" :: "r"(0ull));

    vmm::set_pat();

    for(;;)
        asm ("pause");
}

void boot_aps() {
    memcpy8(reinterpret_cast<uint8_t*>(0x1000 + vmm::high_vma),
            reinterpret_cast<uint8_t*>(smp_core_init_begin),
            reinterpret_cast<size_t>(smp_core_init_end) - reinterpret_cast<size_t>(smp_core_init_begin));

    x86::idtr idtr;
    asm volatile ("sidt %0" :: "m"(idtr));

    x86::gdtr gdtr;
    asm volatile ("sgdt %0" :: "m"(gdtr));

    uint32_t current_apic_id = apic::lapic->read(apic::lapic->id_reg());

    vmm::kernel_mapping->map_page_raw(0, 0, 0x3, 0x3 | (1 << 7) | (1 << 8), -1); 

    for(size_t i = 0; i < madt0_list.size(); i++) {
        madt0 madt0_entry = madt0_list[i];
        uint32_t apic_id = madt0_entry.apic_id;

        cpu new_cpu = { 0,
                        pmm::alloc(2) + 0x2000 + vmm::high_vma,
                        0,
                        -1,
                        -1,
                        vmm::kernel_mapping,
                        NULL
                      };

        cpus.push(new_cpu);

        if(apic_id == current_apic_id) {
            wrmsr(msr_gs_base, reinterpret_cast<size_t>(&cpus.data()[i]));
            continue;
        }

        if(madt0_entry.flags == 1) {
            prep_core(  new_cpu.kernel_stack,
                        reinterpret_cast<uint64_t>(vmm::kernel_mapping->highest_raw),
                        reinterpret_cast<uint64_t>(core_bootstrap),
                        reinterpret_cast<uint64_t>(&idtr),
                        reinterpret_cast<uint64_t>(&gdtr),
                        i);
                    
            apic::lapic->send_ipi(apic_id, 0x500); // MT = 0b101 for init ipi
            apic::lapic->send_ipi(apic_id, 0x600 | 1); // MT = 0b11 for startup, vec = 1 for 0x1000

            ksleep(20);
        }
    }

    vmm::kernel_mapping->unmap_page(0);
}

cpu &core_local() {
    int index = 0;
    asm volatile ("mov %%gs:0, %0" : "=r"(index));
    return cpus[index];
}

}
