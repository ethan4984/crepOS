#ifndef ELF_HPP_
#define ELF_HPP_

#include <mm/vmm.hpp>
#include <string.hpp>

namespace elf {

constexpr size_t elf_signature = 0x464C457F;
constexpr size_t elf64 = 0x2;

constexpr size_t ei_class = 0x4;
constexpr size_t ei_data = 0x5;
constexpr size_t ei_version = 0x6;
constexpr size_t ei_osabi = 0x7;

constexpr size_t abi_system_v = 0x0;
constexpr size_t abi_linux = 0x3;

constexpr size_t little_endian = 0x1;
constexpr size_t mach_x86_64 = 0x3e;

struct aux {
    uint64_t at_entry;
    uint64_t at_phdr;
    uint64_t at_phent;
    uint64_t at_phnum;
};

constexpr size_t at_entry = 10;
constexpr size_t at_phdr = 20;
constexpr size_t at_phent = 21;
constexpr size_t at_phnum = 22;

struct elf64_phdr {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

constexpr size_t pt_null = 0x0;
constexpr size_t pt_load = 0x1;
constexpr size_t pt_dynamic = 0x2;
constexpr size_t pt_interp = 0x3;
constexpr size_t pt_note = 0x4;
constexpr size_t pt_shlib = 0x5;
constexpr size_t pt_phdr = 0x6;
constexpr size_t pt_lts = 0x7;
constexpr size_t pt_loos = 0x60000000;
constexpr size_t pt_hois = 0x6fffffff;
constexpr size_t pt_loproc = 0x70000000;
constexpr size_t pt_hiproc = 0x7fffffff;

struct elf64_shdr {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addr_align;
    uint64_t sh_entsize;
};

struct file {
    file(vmm::pmlx_table *page_map, aux *aux_cur, int fd, uint64_t base, lib::string **ld_path);

    struct [[gnu::packed]] {
        uint8_t ident[16];
        uint16_t type;
        uint16_t machine;
        uint32_t version;
        uint64_t entry;
        uint64_t phoff;
        uint64_t shoff;
        uint32_t flags;
        uint16_t hdr_size;
        uint16_t phdr_size;
        uint16_t ph_num;
        uint16_t shdr_size;
        uint16_t sh_num;
        uint16_t shstrndx;
    } hdr;
};

};

#endif
