#ifndef AHCI_HPP_
#define AHCI_HPP_

#include <drivers/pci.hpp>

namespace ahci {

constexpr size_t sector_size = 0x200;

constexpr size_t sata_ata = 0x101;
constexpr size_t sata_atapi = 0xeb140101;
constexpr size_t sata_semb = 0xc33C0101;
constexpr size_t sata_pm = 0x96690101;

constexpr size_t hba_cmd_st = 0x1;
constexpr size_t hba_cmd_fre = 0x10;
constexpr size_t hba_cmd_fr = 0x4000;
constexpr size_t hba_cmd_cr = 0x8000;

constexpr size_t fis_reg_h2d = 0x27;
constexpr size_t fis_reg_d2h = 0x34;
constexpr size_t fis_dma_enable = 0x39;
constexpr size_t fis_dma_setup = 0x41;
constexpr size_t fis_data = 0x46;
constexpr size_t fis_bist = 0x58;
constexpr size_t fis_pio_setup = 0x5f;
constexpr size_t fis_device_bits = 0xa1;

struct [[gnu::packed]] port_regs {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t reserved0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sstl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t devslp;
    uint32_t reserved1[11];
    uint32_t vs[10];
};

struct [[gnu::packed]] bar_regs {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is; 
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_ports;
    uint32_t em_lock;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;
    uint32_t reserved[29];
    uint32_t vendor[24];
    port_regs port[];
};

struct [[gnu::packed]] hba_cmd {
    uint8_t cfl : 5;
    uint8_t a : 1;
    uint8_t w : 1;
    uint8_t p : 1;
    uint8_t r : 1;
    uint8_t b : 1;
    uint8_t c : 1;
    uint8_t reserved0 : 1;
    uint8_t pmp : 4;
    uint16_t prdtl;
    volatile uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsv1[4];
};

struct fis_h2d {
    uint8_t fis_type;
    uint8_t pmport : 4;
    uint8_t reserved0 : 3;
    uint8_t c : 1;
    uint8_t command;
    uint8_t featurel;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureh;
    uint8_t countl;
    uint8_t counth;
    uint8_t icc;
    uint8_t control;
    uint32_t reserved1;
};

struct fis_d2h {
    uint8_t fis_type;
    uint8_t pmport : 4;
    uint8_t reserved0 : 2;
    uint8_t i : 1;
    uint8_t reserved1 : 1;
    uint8_t status;
    uint8_t error;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved2;
    uint8_t countl;
    uint8_t counth;
    uint16_t reserved3;
    uint32_t reserved4;
};

struct hba_prdt {
    uint32_t dba;
    uint32_t dbau;
    uint32_t reserved0;
    uint32_t dbc : 22;
    uint32_t reserved1 : 9;
    uint32_t i : 1;
};

struct hba_command_table {
    uint8_t cfis[64];
    uint8_t acmd[16];
    uint8_t reserved[48];
    struct hba_prdt PRDT[1];   
};

class controller {
public:
    controller(pci::device pci_device);

    size_t sector_cnt;
    size_t port_cnt;
    size_t cmd_slots;
private:
    pci::device pci_device;
    pci::bar bar;
    volatile bar_regs *ghc_regs;
};

class device {
public:
    device(controller *parent, volatile port_regs *regs);
    device() = default;

    int find_cmd_slot();
    void send_cmd(size_t cmd_slot);
    void lba_rw(size_t start, size_t cnt, void *buffer, bool w);

    size_t sector_cnt;
private:
    controller *parent;
    volatile port_regs *regs; 
};

inline lib::vector<controller*> controller_list;

}

#endif
