#ifndef PCI_HPP_
#define PCI_HPP_

#include <vector.hpp>

namespace pci {

struct bar {
    size_t base;
    size_t size;
};

struct device {
    device(uint8_t bus, uint8_t device, uint8_t func);
    device() = default;

    void write(uint8_t off, uint32_t data); 
    uint32_t read(uint8_t off);
    void become_bus_master();
    void enable_mmio();
    int set_msi(uint8_t vec);
    int get_bar(bar &ret, size_t num);

    uint8_t bus;
    uint8_t device_code;
    uint8_t func;
    uint8_t class_code;
    uint8_t sub_class;
    uint8_t prog_if;
    uint16_t device_id;
    uint16_t vendor_id;
};

inline lib::vector<device> device_list;

void scan_devices();

}

#endif
