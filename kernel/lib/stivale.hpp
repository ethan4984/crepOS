#ifndef STIVALE_HPP_
#define STIVALE_HPP_

#include <stdint.h>

struct stivale_module {
    uint64_t begin;
    uint64_t end;
    char string[3];
};

struct stivale_mmap {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t unused;
};

struct stivale {
    char *cmdline;
    uint64_t mmap_addr;
    uint64_t mmap_cnt;
    uint64_t fb_addr;
    uint16_t fb_pitch;
    uint16_t fb_width;
    uint16_t fb_height;
    uint16_t fb_bpp;
    uint64_t rsdp;
    uint64_t module_cnt;
    stivale_module modules[];
};

#endif
