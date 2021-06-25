#include <mm/vmm.hpp>
#include <mm/pmm.hpp>
#include <mm/slab.hpp>

#include <int/idt.hpp>
#include <int/gdt.hpp>
#include <int/apic.hpp>

#include <drivers/hpet.hpp>
#include <drivers/pci.hpp>

#include <sched/scheduler.hpp>
#include <sched/smp.hpp>

#include <fs/vfs.hpp>

#include <stivale.hpp>
#include <debug.hpp>
#include <vector.hpp>
#include <map.hpp>

extern "C" void __cxa_pure_virtual() { while (1); }
extern "C" void _init();

void kernel_thread() {
    print("Welcome to the kernel thread bois\n");
    for(;;)
        asm ("pause");
}

extern "C" int main(size_t stivale_phys) {
    cpuid_state cpu_id = cpuid(7, 0);

    if(cpu_id.rcx & (1 << 16)) {
        vmm::high_vma = 0xff00000000000000;
    }

    stivale *stivale_virt = reinterpret_cast<stivale*>(stivale_phys + vmm::high_vma);

    pmm::init(stivale_virt);

    kmm::cache(NULL, 0, 32);
    kmm::cache(NULL, 0, 64);
    kmm::cache(NULL, 0, 128);
    kmm::cache(NULL, 0, 256);
    kmm::cache(NULL, 0, 512);
    kmm::cache(NULL, 0, 1024);
    kmm::cache(NULL, 0, 2048);
    kmm::cache(NULL, 0, 4096);
    kmm::cache(NULL, 0, 8192);
    kmm::cache(NULL, 0, 16384);
    kmm::cache(NULL, 0, 32768);
    kmm::cache(NULL, 0, 65536);
    kmm::cache(NULL, 0, 131072);
    kmm::cache(NULL, 0, 262144);

    vmm::init();

    _init();

    x86::gdt_init();
    x86::idt_init();

    new x86::tss;

    rsdp_ptr = reinterpret_cast<rsdp*>(stivale_virt->rsdp + vmm::high_vma);

    if(rsdp_ptr->xsdt_addr) {
        xsdt_ptr = reinterpret_cast<xsdt*>(rsdp_ptr->xsdt_addr + vmm::high_vma);
        print("[ACPI] xsdt found at {x}\n", reinterpret_cast<size_t>(xsdt_ptr));
    } else if(rsdp_ptr->rsdt_addr) {
        rsdt_ptr = reinterpret_cast<rsdt*>(rsdp_ptr->rsdt_addr + vmm::high_vma);
        print("[ACPI] rsdt found at {x}\n", reinterpret_cast<size_t>(rsdt_ptr));
    }

    init_hpet();

    apic::init();
    smp::boot_aps();

    pci::scan_devices();

    apic::timer_calibrate(100);

    ssize_t pid = sched::create_task(-1, NULL);
    sched::create_thread(pid, (size_t)kernel_thread, 0x8);

    asm ("sti");

    for(;;)
        asm ("pause");
}
