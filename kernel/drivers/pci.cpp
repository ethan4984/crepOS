#include <drivers/pci.hpp>
#include <drivers/nvme/nvme.hpp>
#include <drivers/ahci/ahci.hpp>
#include <drivers/xhci/xhci.hpp>
#include <int/apic.hpp>

#define GET_CLASS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0x8) >> 24)

#define GET_SUB_CLASS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0x8) >> 16)

#define GET_PROG_IF(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0x8) >> 8)

#define GET_DEVICE_ID(BUS, DEVICE, FUNC) \
    (uint16_t)(pci::raw_read(BUS, DEVICE, FUNC, 0) >> 16)

#define GET_VENDOR_ID(BUS, DEVICE, FUNC) \
    (uint16_t)(pci::raw_read(BUS, DEVICE, FUNC, 0))

#define GET_HEADER_TYPE(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0xc) >> 16)

#define GET_SECONDARY_BUS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0x18) >> 8)

namespace pci {

static inline uint32_t raw_read(uint8_t bus, uint8_t device_code, uint8_t func, uint8_t off) {
    outd(0xcf8, (1 << 31) | // enable
                ((uint32_t)bus << 16) | // bus number
                (((uint32_t)device_code & 31) << 11) | // device number
                (((uint32_t)func & 7) << 8) | // function number
                ((uint32_t)off & ~(3)));
    return ind(0xcfc);
}

static inline void raw_write(uint32_t data, uint8_t bus, uint8_t device_code, uint8_t func, uint8_t off) {
    outd(0xcf8, (1 << 31) | // enable
                ((uint32_t)bus << 16) | // bus number
                (((uint32_t)device_code & 31) << 11) | // device number
                (((uint32_t)func & 7) << 8) | // function number
                ((uint32_t)off & ~(3)));
    outd(0xcfc, data);
}

static void scan_bus(uint8_t bus) {
    for(uint8_t dev = 0; dev < 32; dev++) { 
        if(GET_VENDOR_ID(bus, dev, 0) == 0xffff)
            continue;

        device_list.push(device(bus, dev, 0));

        if(GET_HEADER_TYPE(bus, dev, 0) & (1 << 7)) { // multi function device
            for(uint8_t func = 1; func < 8; func++) 
                if(GET_VENDOR_ID(bus, dev, func) != 0xffff) 
                    device_list.push(device(bus, dev, func));
        }
    }
}

device::device(uint8_t bus, uint8_t device_code, uint8_t func) : bus(bus),
                                                                 device_code(device_code),
                                                                 func(func),
                                                                 class_code(GET_CLASS(bus, device_code, func)),
                                                                 sub_class(GET_SUB_CLASS(bus, device_code, func)),
                                                                 prog_if(GET_PROG_IF(bus, device_code, func)),
                                                                 device_id(GET_DEVICE_ID(bus, device_code, func)),
                                                                 vendor_id(GET_VENDOR_ID(bus, device_code, func)) {
    if((GET_HEADER_TYPE(bus, device_code, func) & ~(1 << 7)) == 1) { // pci to pci bridge
        scan_bus(GET_SECONDARY_BUS(bus, device_code, func));
    }
}

uint32_t device::read(uint8_t off) {
    return raw_read(bus, device_code, func, off);
}

void device::write(uint8_t off, uint32_t data) {
    raw_write(data, bus, device_code, func, off);
}

void device::become_bus_master() {
    write(0x4, read(0x4) | (1 << 2));    
}

void device::enable_mmio() {
    write(0x4, read(0x4) | (1 << 1));
}

int device::get_bar(bar &ret, size_t num) {
    if(GET_HEADER_TYPE(bus, device_code, func) != 0) // ensure hdr type == 0
        return -1;

    if(num > 5) 
        return -1;

    size_t bar_off = 0x10 + num * 4;
    size_t bar_low = read(bar_off);
    size_t is_mmio = !(bar_low & 1);

    write(bar_off, ~0);
    size_t bar_size_low = read(bar_off);
    write(bar_off, bar_low); 

    if(((bar_low >> 1) & 0b11) == 0b10) { // is 64 bit
        size_t bar_high = read(bar_off + 4);

        write(bar_off + 4, ~0);
        size_t bar_size_high = read(bar_off + 4);
        write(bar_off + 4, bar_high); 

        size_t size = ((bar_size_high << 32) | bar_size_low) & ~(is_mmio ? 0b1111 : 0b11);
        size = ~size + 1;

        size_t base = ((bar_high << 32) | bar_low) & ~(is_mmio ? 0b1111 : 0b11);

        ret = { base, size };

        return 0;
    }

    size_t size = bar_size_low & is_mmio ? 0b1111 : 0b11;
    size = ~size + 1; 

    ret = { bar_low, size };

    return 0;
}

int device::set_msi(uint8_t vec) {
    uint32_t reg4 = read(0x4);
    uint32_t off = reg4 >> 16 & (1 << 4) ? read(0x34) : 0;

    while(off) {
        uint8_t id = read(off);

        switch(id) { 
            case 0x5:
                goto found;
        }

        off = read(off + 1);
    }

    print("[PCI] Device Does not support MSI\n");
    return -1;

found:
    uint16_t message_control = read(off + 0x2) >> 16;

    if(message_control & (1 << 7)) { // 64 bit
        uint32_t addr = read(off + 0x4);
        uint32_t data = read(off + 0xb) >> 16;

        data |= vec << 0;
        data |= 0 << 11; // fixed delivery mode

        addr |= 0xfee << 20; 
        addr |= apic::lapic->read(apic::lapic->id_reg()) << 12; // destination id

        write(off + 0x4, addr);
        write(off + 0xb, data);
    } else {
        uint32_t addr = read(off + 0x4);
        uint32_t data = read(off + 0x8);

        data |= vec << 0;
        data |= 0 << 11; // fixed delivery mode

        addr |= 0xfee << 20; 
        addr |= apic::lapic->read(apic::lapic->id_reg()) << 12; // destination id

        write(off + 0x4, addr);
        write(off + 0x8, data);
    }

    message_control |= 1; // msi enable
    write(off + 0x2, message_control);

    return 0;
}

void scan_devices() {
    for(int bus = 0; bus < 256; bus++) {
        scan_bus(bus);
    }

    for(size_t i = 0; i < device_list.size(); i++) {
        device pci_device = device_list[i];

        print("[PCI] device: {x}\n", i);
        print("[PCI] \tVendor: {x} on [bus] {x} [function] {x}\n",  pci_device.vendor_id,
                                                                    pci_device.bus,
                                                                    pci_device.func);
        print("[PCI] \tdevice type: [class] {x} [subclass] {x} [prog_IF] {x} [device ID] {x}\n",    pci_device.class_code,
                                                                                                    pci_device.sub_class,
                                                                                                    pci_device.prog_if,
                                                                                                    pci_device.device_id);
        switch(pci_device.class_code) {
            case 1: { // mass storage controller
                switch(pci_device.sub_class) {
                    case 6: { // serial ata
                        //ahci::controller *new_device = new ahci::controller(pci_device);
                        new ahci::controller(pci_device);
                        break;
                    }
                    case 8: { // nvme
                        nvme::device *new_device = new nvme::device(pci_device);
                        nvme::device_list.push(new_device);
                    }
                }
                break;
            }
            case 0xc: { // serial bus controller
                switch(pci_device.sub_class) {
                    case 0x3: // xhci
                        new xhci::controller(pci_device);
                }
            }
        }
    }
}

}
