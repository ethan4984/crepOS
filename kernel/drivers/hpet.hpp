#ifndef HPET_HPP_
#define HPET_HPP_

#include <acpi/rsdt.hpp>

struct [[gnu::packed]] hpet_table {
    acpihdr acpihdr_ptr;
    uint8_t hardware_rev_id;
    uint8_t info;
    uint16_t pci_id;
    uint8_t address_space_id;
    uint8_t register_width;
    uint8_t register_offset;
    uint8_t reserved;
    uint64_t address;
    uint8_t hpet_num;
    uint16_t minim_ticks;
    uint8_t page_protection;
};

struct [[gnu::packed]] hpet {
    uint64_t capabilities;
    uint64_t unused0;
    uint64_t general_config;
    uint64_t unused1;
    uint64_t int_status;
    uint64_t unused2;
    uint64_t unused3[24];
    uint64_t counter_value;
    uint64_t unused4;
};

void ksleep(size_t ms);
void init_hpet();

#endif
