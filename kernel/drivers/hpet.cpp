#include <drivers/hpet.hpp>

static hpet_table *hpet_table_ptr;
static hpet *hpet_ptr;

void ksleep(uint64_t ms) {
    uint64_t ticks = hpet_ptr->counter_value + (ms * 1000000000000) / ((hpet_ptr->capabilities >> 32) & 0xffffffff); 
    for(;hpet_ptr->counter_value < ticks;); 
}

void init_hpet() {
    hpet_table_ptr = find_SDT<hpet_table>("HPET");
    hpet_ptr = reinterpret_cast<hpet*>(hpet_table_ptr->address + vmm::high_vma);
    hpet_ptr->general_config = 1;
}
