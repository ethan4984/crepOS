#include <drivers/xhci/xhci.hpp>

namespace xhci {

controller::controller(pci::device pci_device) : pci_device(pci_device) {
    switch(pci_device.prog_if) {
        case 0x0:
            print("[USB] UHCI controller detcted\n");
            return;
        case 0x10:
            print("[USB] OHCI controller detcted\n");
            return;
        case 0x20:
            print("[USB] EHCI controller detected\n");
            return;
        case 0x30:
            print("[USB] XHCI controller detected\n");
            break;
        case 0x80:
            print("[USB] Controller type unspecified\n"); 
            return;
        case 0xfe:
            return;
    }

    pci_device.become_bus_master();
    pci_device.enable_mmio();
    pci_device.get_bar(bar, 0);

    cap = reinterpret_cast<volatile cap_regs*>(bar.base + vmm::high_vma);
    operation = reinterpret_cast<volatile operation_regs*>(bar.base + cap->cap_length + vmm::high_vma);
    runtime = reinterpret_cast<volatile runtime_regs*>(bar.base + (cap->runtime_reg_space_offset & ~0x1f) + vmm::high_vma);
    db = reinterpret_cast<volatile db_regs*>(bar.base + (cap->doorbell_offset & ~0x3) + vmm::high_vma);
    extended_cap = reinterpret_cast<volatile uint32_t*>(bar.base + (cap->cap_parms1 >> 16 & 0xffff) * 4 + vmm::high_vma);

    auto major_version = cap->interface_vs >> 8 & 0xff;
    auto minor_version = cap->interface_vs >> 4 & 0xf;
    auto tertiary_version = cap->interface_vs & 0xf;

    print("[XHCI] HCI Controller Detcted {}.{}.{}\n", major_version, minor_version, tertiary_version);

    max_device_slots = cap->sparms1 & 0xff;
    max_interrupts =  cap->sparms1 >> 8 & 0xff;
    max_ports = cap->sparms1 >> 24 & 0xff;

    print("[XHCI] Max device slots {}\n", max_device_slots);
    print("[XHCI] Max interrupts {}\n", max_interrupts);
    print("[XHCI] Max ports {}\n", max_ports);

    if(cap->cap_parms1 & 0x1) {
        print("[XHCI] 64 bit addressing supported\n");
    } else {
        print("[XHCI] 64 bit addressing not supported\n");
    }

    max_page_size = [&]() -> size_t {
        ssize_t highest_bit = 0; 
        for(size_t i = 0; i < 16; i++) {
            if(operation->page_size & (1 << i)) {
                highest_bit = i;
            }
        }

        return pow(2, 12 + highest_bit);
    } ();

    print("[XHCI] Max page size {x}\n", max_page_size);

    for(size_t i = 0;;) {
        auto cap_id = extended_cap[i] & 0xff;
        auto next_cap = extended_cap[i] >> 8 & 0xff;
        auto version_minor = extended_cap[i] >> 16 & 0xff;
        auto version_major = extended_cap[i] >> 24 & 0xff;

        switch(cap_id) {
            case 1:
                break;
            case 2:
                break;
            defualt:
                print("[XHCI] Unhandled protocol with cap id {}\n", cap_id);
        }

        if(!next_cap) 
            break;

        i += next_cap; 
    }
}

}
