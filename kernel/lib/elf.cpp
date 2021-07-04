#include <elf.hpp>
#include <mm/mmap.hpp>

namespace elf {

file::file(vmm::pmlx_table *page_map, aux *aux_cur, fs::fd &file, uint64_t base, lib::string **ld_path) {
    auto elf_status = [&, this]() {
        file.read((void*)&hdr, sizeof(hdr));

        if(*(uint32_t*)&hdr != elf_signature) 
            return -1;

        if(hdr.ident[ei_osabi] != abi_system_v && hdr.ident[ei_osabi] != abi_linux) return -1;
        if(hdr.ident[ei_data] != little_endian) return -1;
        if(hdr.ident[ei_class] != elf64) return -1;
        if(hdr.machine != mach_x86_64 && hdr.machine != 0) return -1;

        return 0;
    } ();

    if(elf_status == -1) 
        return;

    elf64_phdr *phdr = new elf64_phdr[hdr.ph_num];

    file.seek(hdr.phoff, seek_set);
    file.read(phdr, sizeof(elf64_phdr) * hdr.ph_num);

    aux_cur->at_phdr = 0;
    aux_cur->at_phent = sizeof(elf64_phdr);
    aux_cur->at_phnum = hdr.ph_num;

    for(size_t i = 0; i < hdr.ph_num; i++) {
        if(phdr[i].p_type == pt_interp) {
            if(ld_path == NULL) 
                continue;

            *ld_path = new lib::string;
            char *path = (char*)kmm::calloc(phdr[i].p_filesz + 1);

            file.seek(phdr[i].p_offset, seek_set);
            file.read(path, phdr[i].p_filesz);

            **ld_path = lib::string(path);
            delete path;

            continue;
        } else if(phdr[i].p_type == pt_phdr) { 
            aux_cur->at_phdr = base + phdr[i].p_vaddr;
            continue;
        } else if(phdr[i].p_type != pt_load) {
            continue;
        }

        size_t misalignment = phdr[i].p_vaddr & (vmm::page_size - 1);
        size_t page_cnt = div_roundup(misalignment + phdr[i].p_memsz, vmm::page_size);

        mm::mmap(page_map, (void*)(phdr[i].p_vaddr + base), page_cnt * vmm::page_size, 0x3 | (1 << 2), mm::map_anonymous | mm::map_fixed, 0, 0);

        file.seek(phdr[i].p_offset, seek_set);
        file.read((void*)(phdr[i].p_vaddr + base), phdr[i].p_filesz);

    }

    aux_cur->at_entry = base + hdr.entry;
}

}
