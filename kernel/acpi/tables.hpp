#ifndef TABLES_HPP_
#define TABLES_HPP_

#include <cstdint>
#include <cstddef>

struct [[gnu::packed]] acpihdr {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEM_table_id[8];
    uint32_t OEM_revision;
    uint32_t creator_ID;
    uint32_t creator_revision;
};

struct [[gnu::packed]] rsdp {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision; 
    uint32_t rsdt_addr;
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t ext_checksum;
    uint8_t reserved[3];
};

struct [[gnu::packed]] rsdt {
    acpihdr acpihdr_ptr;
    uint32_t acpiptr[];
};

struct [[gnu::packed]] xsdt {
    acpihdr acpihdr_ptr;
    uint64_t acpiptr[];
};

#endif
