#ifndef XHCI_HPP_
#define XHCI_HPP_

#include <drivers/pci.hpp>

namespace xhci {

struct [[gnu::packed]] cap_regs {
    uint8_t cap_length;
    uint8_t reserved0;
    uint16_t interface_vs;
    uint32_t sparms1;
    uint32_t sparms2;
    uint32_t sparms3;
    uint32_t cap_parms1;
    uint32_t doorbell_offset;
    uint32_t runtime_reg_space_offset;
    uint32_t reserved[];
};

struct [[gnu::packed, gnu::aligned(4)]] port_regs {
    uint32_t status_control;
    uint32_t pm_status_control;
    uint32_t link_info;
    uint32_t lpm_control;
};

struct [[gnu::packed, gnu::aligned(8)]] operation_regs {
    uint32_t usb_command;
    uint32_t usb_status;
    uint32_t page_size;
    uint8_t reserved0[0x14 - 0xc];
    uint32_t dov_notification_ctrl;
    uint32_t command_ring_control;
    uint8_t reserved1[0x30 - 0x20];
    uint64_t device_context_addr;
    uint32_t config;
    uint8_t reserved2[0x400 - 0x3c];
    port_regs prs[];
};

struct [[gnu::packed, gnu::aligned(8)]] irs {
    uint32_t interrupter_management;
    uint32_t interrupter_moderation;
    uint32_t event_ring_tbl_size;
    uint32_t reserved;
    uint64_t event_ring_segment_addr;
    uint64_t event_ring_dequeue_addr;
};

struct [[gnu::packed, gnu::aligned(8)]] runtime_regs {
    uint32_t microframe_index;
    uint8_t reserved[28];
    volatile irs irs_list[1024];
};

struct [[gnu::packed, gnu::aligned(8)]] db_regs {
    uint32_t db[256];
};

class controller {
public:
    controller(pci::device pci_device);
    controller();

    size_t max_device_slots; 
    size_t max_interrupts;
    size_t max_ports;
    size_t page_size;
private:
    pci::device pci_device;
    pci::bar bar;

    volatile cap_regs *cap;
    volatile operation_regs *operation;
    volatile runtime_regs *runtime;
    volatile db_regs *db;
    volatile uint32_t *extended_cap;

    size_t max_page_size;
};

inline lib::vector<controller*> controller_list;

};

#endif
