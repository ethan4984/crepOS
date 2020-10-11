#pragma once

#include <kernel/acpi/rsdp.h>

struct [[gnu::packed]] madt_t {
    acpihdr_t acpihdr;
    uint32_t lapicAddr;
    uint32_t flags;
    uint8_t entries[];
};
    
struct [[gnu::packed]] madtEntry0_t {
    uint8_t acpiProcessorID;
    uint8_t apicID;
    uint32_t flags;
};

struct [[gnu::packed]] madtEntry1_t {
    uint8_t ioapicID;
    uint8_t reserved;
    uint32_t ioapicAddr;
    uint32_t gsiBase;
};

struct [[gnu::packed]] madtEntry2_t {
    uint8_t busSrc;
    uint8_t irqSrc;
    uint32_t gsi;
    uint16_t flags;
};

struct [[gnu::packed]] madtEntry4_t {
    uint8_t acpiProcessorID;
    uint16_t flags; 
    uint8_t lint;
};

struct [[gnu::packed]] madtEntry5_t {
    uint16_t reserved;
    uint8_t lapicOverride;
};

class madtInfo_t {
public:
    void madtInit();

    void printMADT();

    uint8_t madtEntry0Count;
    uint8_t madtEntry1Count;
    uint8_t madtEntry2Count;
    uint8_t madtEntry4Count;
    uint8_t madtEntry5Count;

    madtEntry0_t *madtEntry0;
    madtEntry1_t *madtEntry1;
    madtEntry2_t *madtEntry2;
    madtEntry4_t *madtEntry4;
    madtEntry5_t *madtEntry5;

    uint32_t lapicAddr;
};

inline madtInfo_t madtInfo;
