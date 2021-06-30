#ifndef MMAP_HPP_
#define MMAP_HPP_

#include <mm/vmm.hpp>

namespace mm {

constexpr ssize_t map_failed = -1;
constexpr ssize_t map_private = 0x1;
constexpr ssize_t map_shared = 0x2;
constexpr ssize_t map_fixed = 0x4;
constexpr ssize_t map_anonymous = 0x8;

constexpr ssize_t mmap_min_addr = 0x10000;

void *mmap(vmm::pmlx_table *page_map, void *addr, size_t length, int prot, int flags, int fd, ssize_t off);
ssize_t munmap(vmm::pmlx_table *page_map, void *addr, size_t length);
void mmap_reserve(vmm::pmlx_table *page_map, void *addr, size_t length);

}

#endif
