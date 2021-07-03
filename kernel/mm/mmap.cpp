#include <mm/mmap.hpp>
#include <mm/slab.hpp>
#include <fs/fd.hpp>
#include <sched/smp.hpp>

namespace mm {

static void *mmap_alloc(vmm::pmlx_table *page_map, void *addr, size_t length, int flags) {
    size_t page_cnt = div_roundup(length, vmm::page_size), offset = 0;

    if(page_map->bitmap == NULL) {
        page_map->bm_size = 0x1000;
        page_map->bitmap = (uint8_t*)kmm::calloc(page_map->bm_size); 
    }

    auto bm_index = [&](void *address) {
        return ((size_t)address - mmap_min_addr) / vmm::page_size;
    };

    if(flags & map_fixed) {
        auto index = bm_index(addr);
        for(size_t i = index; i < index + page_cnt; i++) { 
            bm_set(page_map->bitmap, i);
        }
        return addr;
    }

    for(size_t i = 0, cnt = 0; i < page_map->bm_size; i++) {
        if(bm_test(page_map->bitmap, i)) {
            offset += cnt + 1;
            cnt = 0;
        }

        if(++cnt == page_cnt) {
            addr = (void*)(offset * vmm::page_size + mmap_min_addr); 
            auto index = bm_index(addr); 

            for(size_t i = index; i < index + page_cnt; i++) {
                bm_set(page_map->bitmap, i);
            }

            return addr;
        }
    }

    page_map->bm_size += 0x1000;
    page_map->bitmap = (uint8_t*)kmm::recalloc(page_map->bitmap, page_map->bm_size);

    return mmap_alloc(page_map, addr, length, flags);
}

static ssize_t check_mmap_addr(vmm::pmlx_table *page_map, void *addr, size_t length, int flags) {
    if(addr == NULL) 
        return -1;

    if(flags == map_fixed) 
        return 0;

    if(page_map->bitmap == NULL) {
        page_map->bm_size = 0x1000;
        page_map->bitmap = (uint8_t*)kmm::calloc(page_map->bm_size); 
    }

    auto index = [&](void *address) {
        return ((size_t)address - mmap_min_addr) / vmm::page_size;
    } (addr);

    size_t page_cnt = div_roundup(length, vmm::page_size);

    for(size_t i = index; i < index + page_cnt; i++) {
        if(bm_test(page_map->bitmap, i)) {
            return -1;
        }
    }

    return 0;
}

void *mmap(vmm::pmlx_table *page_map, void *addr, size_t length, int prot, int flags, int fd, [[maybe_unused]] ssize_t off) {
    size_t page_cnt = div_roundup(length, vmm::page_size);

    if(flags == map_anonymous) {
        if(check_mmap_addr(page_map, addr, length, flags) == -1) {
            addr = mmap_alloc(page_map, addr, length, flags);
        }

        print("Mapping address {x} page_cnt {x}\n", (size_t)addr, page_cnt); 

        page_map->map_range((size_t)addr, page_cnt, prot, prot);
    }

    if(flags & map_fixed) {
        page_map->map_range((size_t)addr, page_cnt, prot, prot);
        mmap_alloc(page_map, addr, length, flags);
        if(!(flags & map_anonymous)) {
            fs::fd &fd_back = fs::fd_list[fd]; 
            if(fd_back.backing_fd != -1 || fd_back.status != 0) {
                fd_back.read(addr, length);
            } else {
                fs::fd_list.remove(fd); 
            }
        }
    }

    return addr;
}

ssize_t munmap(vmm::pmlx_table *page_map, void *addr, size_t length) {
    size_t index = ((size_t)addr - mmap_min_addr) / vmm::page_size;
    size_t page_cnt = div_roundup(length, vmm::page_size);

    if(index >= page_map->bm_size) {
        return -1;
    } else if(index + page_cnt > page_map->bm_size) {
        page_cnt = page_map->bm_size - index;
    }

    for(size_t i = index; i < index + page_cnt; i++) {
        bm_clear(page_map->bitmap, i);
    }

    page_map->unmap_range((size_t)addr, page_cnt);

    return 0;
}

extern "C" void syscall_mmap(regs *regs_cur) {
    smp::cpu &cpu = smp::core_local();
    regs_cur->rax = (size_t)mmap(cpu.page_map, (void*)regs_cur->rdi, regs_cur->rsi, (int)regs_cur->rdx | (1 << 2), (int)regs_cur->r10, (int)regs_cur->r8, (ssize_t)regs_cur->r9);
}

extern "C" void syscall_munmap(regs *regs_cur) {
    smp::cpu &cpu = smp::core_local();
    regs_cur->rax = (size_t)munmap(cpu.page_map, (void*)regs_cur->rdi, regs_cur->rsi);
}

}
