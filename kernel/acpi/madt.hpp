#ifndef MADT_HPP_
#define MADT_HPP_

#include <acpi/tables.hpp>
#include <acpi/rsdt.hpp>
#include <vector.hpp>

struct [[gnu::packed]] madt {
    acpihdr acpihdr_ptr;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t entries[];
};

struct [[gnu::packed]] madt0 {
    uint8_t acpi_id; 
    uint8_t apic_id;
    uint32_t flags;
};

struct [[gnu::packed]] madt1 {
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t gsi_base;
};

struct [[gnu::packed]] madt2 {
    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t gsi;
    uint16_t flags;
};

struct [[gnu::packed]] madt4 {
    uint8_t acpi_id;
    uint16_t flags; 
    uint8_t lint;
};

struct [[gnu::packed]] madt5 {
    uint16_t reserved;
    uint8_t lapic_override;
};

inline madt *madt_ptr;

inline lib::vector<madt0> madt0_list;
inline lib::vector<madt1> madt1_list;
inline lib::vector<madt2> madt2_list;
inline lib::vector<madt4> madt4_list;
inline lib::vector<madt5> madt5_list;

#endif
