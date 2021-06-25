#ifndef SMP_HPP_
#define SMP_HPP_

#include <drivers/nvme/nvme.hpp>
#include <mm/vmm.hpp>
#include <vector.hpp>
#include <types.hpp>

namespace smp {

struct cpu {
    uint64_t user_stack;
    uint64_t kernel_stack;
    ssize_t errno;
    pid_t pid;
    tid_t tid;
    vmm::pmlx_table *page_map;
    nvme::queue *nvme_io_queue;
};

void boot_aps();
cpu &core_local();

inline lib::vector<cpu> cpus;

}

#endif
