#ifndef VMM_HPP_
#define VMM_HPP_

#include <memutils.hpp> 
#include <cstdint>
#include <cstddef>
#include <utility>

namespace vmm {

inline size_t high_vma = 0xffff800000000000;
inline size_t kernel_high_vma = 0xffffffff80000000;
inline size_t page_size = 0x1000;

constexpr size_t uc = 0x0;
constexpr size_t wc = 0x1;
constexpr size_t wt = 0x4;
constexpr size_t wp = 0x5;
constexpr size_t wb = 0x6;
constexpr size_t ucm = 0x7;

constexpr size_t pa_wb = 0x0;
constexpr size_t pa_uc = 0x1;
constexpr size_t pa_wc = 0x2;
constexpr size_t pa_wt = 0x3;
constexpr size_t pa_wp = 0x4;
constexpr size_t pa_ucm = 0x5;

constexpr size_t pat_msr = 0x277;

ssize_t set_pat();

class pmle {
public:
    explicit pmle(uint64_t *entry) : entry(entry) { }
    explicit pmle() = default;
    virtual ~pmle() = default;

    virtual constexpr size_t flags_p() = 0;
    virtual constexpr size_t flags_rw() = 0;
    virtual constexpr size_t flags_us() = 0;
    virtual constexpr size_t flags_pwt() = 0;
    virtual constexpr size_t flags_pcd() = 0;
    virtual constexpr size_t flags_a() = 0;
    virtual constexpr size_t flags_d() = 0;
    virtual constexpr size_t flags_pat() = 0;
    virtual constexpr size_t flags_ps() = 0;
    virtual constexpr size_t flags_g() = 0;
    virtual constexpr size_t flags_nx() = 0;

    bool is_present() { return *entry & flags_p(); }
    bool is_read_write() { return *entry & flags_rw(); }
    bool is_user() { return *entry & flags_us(); }
    bool is_write_through() { return *entry & flags_pwt(); }
    bool is_cache_disabled() { return *entry & flags_pcd(); }
    bool is_access() { return *entry & flags_a(); }
    bool is_dirty() { return *entry & flags_d(); }
    bool is_huge() { return *entry & flags_ps(); }
    bool is_global() { return *entry & flags_g(); }
    bool is_pat() { return *entry & flags_pat(); }
    bool is_no_exec() { return *entry & flags_nx(); }

    void set_present(bool bit) { set_bit(flags_p(), bit); }
    void set_read_write(bool bit) { set_bit(flags_rw(), bit); }
    void set_user(bool bit) { set_bit(flags_us(), bit); }
    void set_write_through(bool bit) { set_bit(flags_pwt(), bit); }
    void set_cache_disabled(bool bit) { set_bit(flags_pcd(), bit); }
    void set_huge(bool bit) { set_bit(flags_ps(), bit); }
    void set_global(bool bit) { set_bit(flags_g(), bit); }
    void set_pat(bool bit) { set_bit(flags_pat(), bit); }
    void set_no_exec(bool bit) { set_bit(flags_nx(), bit); }

    void set_pa(ssize_t pa_index) {
        set_pat(bm_test(&pa_index, 2));
        set_cache_disabled(bm_test(&pa_index, 1));
        set_write_through(bm_test(&pa_index, 0));
    }

    uint64_t get_base() { return *entry & ~(0xfff); }
protected:
    void set_bit(uint64_t bit, bool value) {
        if(value) {
            *entry |= bit;
        } else {
            *entry &= ~bit;
        }
    }

    uint64_t *entry;
};

struct pml5e : pmle {
    pml5e(uint64_t *entry) : pmle(entry) { } 
    pml5e() : pmle() { }

    constexpr size_t flags_p() override { return 1 << 0; }
    constexpr size_t flags_rw() override { return 1 << 1; }
    constexpr size_t flags_us() override { return 1 << 2; }
    constexpr size_t flags_pwt() override { return 1 << 3; }
    constexpr size_t flags_pcd() override  { return 1 << 4; }
    constexpr size_t flags_a() override { return 1 << 5; }
    constexpr size_t flags_nx() override { return 1ull << 63; }
private:
    constexpr size_t flags_d() override { return 0; }
    constexpr size_t flags_ps() override { return 0; }
    constexpr size_t flags_g() override { return 0; }
    constexpr size_t flags_pat() override { return 0; }
};

struct pml4e : pmle {
    pml4e(uint64_t *entry) : pmle(entry) { } 
    pml4e() : pmle() { }

    constexpr size_t flags_p() override { return 1 << 0; }
    constexpr size_t flags_rw() override { return 1 << 1; }
    constexpr size_t flags_us() override { return 1 << 2; }
    constexpr size_t flags_pwt() override { return 1 << 3; }
    constexpr size_t flags_pcd() override { return 1 << 4; }
    constexpr size_t flags_a() override { return 1 << 5; }
    constexpr size_t flags_nx() override { return 1ull << 63; }
private:
    constexpr size_t flags_d() override { return 0; }
    constexpr size_t flags_ps() override { return 0; }
    constexpr size_t flags_g() override { return 0; }
    constexpr size_t flags_pat() override { return 0; }
};

struct pml3e : pmle {
    pml3e(uint64_t *entry) : pmle(entry) { } 
    pml3e() : pmle() { }

    constexpr size_t flags_p() override { return 1 << 0; }
    constexpr size_t flags_rw() override { return 1 << 1; }
    constexpr size_t flags_us() override { return 1 << 2; }
    constexpr size_t flags_pwt() override { return 1 << 3; }
    constexpr size_t flags_pcd() override { return 1 << 4; }
    constexpr size_t flags_a() override { return 1 << 5; }
    constexpr size_t flags_d() override { return 1 << 6; }
    constexpr size_t flags_ps() override { return 1 << 7; }
    constexpr size_t flags_g() override { return 1 << 8; }
    constexpr size_t flags_pat() override { return 1 << 12; }
    constexpr size_t flags_nx() override { return 1ull << 63; }
};

struct pml2e : pmle {
    pml2e(uint64_t *entry) : pmle(entry) { } 
    pml2e() : pmle() { }

    constexpr size_t flags_p() override { return 1 << 0; }
    constexpr size_t flags_rw() override { return 1 << 1; }
    constexpr size_t flags_us() override { return 1 << 2; }
    constexpr size_t flags_pwt() override { return 1 << 3; }
    constexpr size_t flags_pcd() override { return 1 << 4; }
    constexpr size_t flags_a() override { return 1 << 5; }
    constexpr size_t flags_d() override { return 1 << 6; }
    constexpr size_t flags_ps() override { return 1 << 7; }
    constexpr size_t flags_g() override { return 1 << 8; }
    constexpr size_t flags_pat() override { return 1 << 12; }
    constexpr size_t flags_nx() override { return 1ull << 63; }
};

struct pml1e : pmle {
    pml1e(uint64_t *entry) : pmle(entry) { } 
    pml1e() : pmle() { }

    constexpr size_t flags_p() override { return 1 << 0; }
    constexpr size_t flags_rw() override { return 1 << 1; }
    constexpr size_t flags_us() override { return 1 << 2; }
    constexpr size_t flags_pwt() override { return 1 << 3; }
    constexpr size_t flags_pcd() override { return 1 << 4; }
    constexpr size_t flags_a() override { return 1 << 5; }
    constexpr size_t flags_d() override { return 1 << 6; }
    constexpr size_t flags_pat() override { return 1 << 7; }
    constexpr size_t flags_g() override { return 1 << 8; }
    constexpr size_t flags_nx() override { return 1ull << 63; }
private:
    constexpr size_t flags_ps() override { return 0; }
};

struct pmlx_table {
    explicit pmlx_table(uint64_t *highest) : highest_raw(highest), lock(0) { }
    explicit pmlx_table() : highest_raw(0), lock(0) { } 

    virtual void map_range(uint64_t vaddr, size_t cnt, size_t flags, ssize_t pa) = 0;
    virtual void unmap_range(uint64_t vaddr, size_t cnt) = 0;

    virtual void map_page_raw(uint64_t vaddr, uint64_t paddr, uint64_t flags1, uint64_t flags0, ssize_t pa) = 0;
    virtual void map_page(uint64_t vaddr, uint64_t flags, ssize_t pa) = 0;
    virtual void unmap_page(uint64_t vaddr) = 0;

    void init() {
        asm volatile ("mov %0, %%cr3" :: "r" (reinterpret_cast<uint64_t>(highest_raw) - high_vma) : "memory");
    }

    uint64_t *highest_raw;
    uint64_t lock;
};

struct pml4_table : pmlx_table {
    pml4_table(uint64_t *highest) : pmlx_table(highest) { } 
    pml4_table() : pmlx_table() { } 

    void map_range(uint64_t vaddr, size_t cnt, size_t flags, ssize_t pa);
    void unmap_range(uint64_t vaddr, size_t cnt);

    void map_page_raw(uint64_t vaddr, uint64_t paddr, uint64_t flags1, uint64_t flags0, ssize_t pa);
    void map_page(uint64_t vaddr, uint64_t flags, ssize_t pa);
    void unmap_page(uint64_t vaddr);

    class virtual_address {
    public:
        virtual_address(uint64_t *pml4_raw, uint64_t vaddr, uint64_t paddr);
        virtual_address(uint64_t *pml4_raw, uint64_t vaddr);
        virtual_address() : pml4_raw(0) { }
        ~virtual_address() { delete _pml4e; delete _pml3e; delete _pml2e; delete _pml1e; }

        void map(uint64_t pml4_flags, uint64_t pml3_flags, uint64_t pml2_flags, uint64_t pml1_flags, ssize_t pa);
        uint64_t unmap();

        void compute_indices() {
            pml4_idx = (vaddr >> 39) & 0x1ff;
            pml3_idx = (vaddr >> 30) & 0x1ff;
            pml2_idx = (vaddr >> 21) & 0x1ff;
            pml1_idx = (vaddr >> 12) & 0x1ff;
        }

        pml4e *_pml4e;
        pml3e *_pml3e;
        pml2e *_pml2e;
        pml1e *_pml1e;

        uint64_t page_length;
    private:
        uint64_t vaddr;
        uint64_t paddr;

        size_t pml4_idx;
        size_t pml3_idx;
        size_t pml2_idx;
        size_t pml1_idx;

        uint64_t *pml4_raw;
        uint64_t *pml3_raw;
        uint64_t *pml2_raw;
        uint64_t *pml1_raw;
    };
};

struct pml5_table : pmlx_table {
    pml5_table(uint64_t *highest) : pmlx_table(highest) { } 
    pml5_table() : pmlx_table() { } 

    void map_range(uint64_t vaddr, size_t cnt, size_t flags, ssize_t pa);
    void unmap_range(uint64_t vaddr, size_t cnt);

    void map_page_raw(uint64_t vaddr, uint64_t paddr, uint64_t flags1, uint64_t flags0, ssize_t pa);
    void map_page(uint64_t vaddr, uint64_t flags, ssize_t pa);
    void unmap_page(uint64_t vaddr);

    class virtual_address {
    public:
        virtual_address(uint64_t *pml5_raw, uint64_t vaddr, uint64_t paddr);
        virtual_address(uint64_t *pml5_raw, uint64_t vaddr);
        virtual_address() : pml5_raw(0) { }
        ~virtual_address() { delete _pml5e; delete _pml4e; delete _pml3e; delete _pml2e; delete _pml1e; }

        void map(uint64_t pml5_flags, uint64_t pml4_flags, uint64_t pml3_flags, uint64_t pml2_flags, uint64_t pml1_flags, ssize_t pa);
        uint64_t unmap();

        void compute_indices() {
            pml5_idx = (vaddr >> 48) & 0x1ff;
            pml4_idx = (vaddr >> 39) & 0x1ff;
            pml3_idx = (vaddr >> 30) & 0x1ff;
            pml2_idx = (vaddr >> 21) & 0x1ff;
            pml1_idx = (vaddr >> 12) & 0x1ff;
        }

        pml5e *_pml5e;
        pml4e *_pml4e;
        pml3e *_pml3e;
        pml2e *_pml2e;
        pml1e *_pml1e;

        uint64_t page_length;
    private:
        uint64_t vaddr;
        uint64_t paddr;

        size_t pml5_idx;
        size_t pml4_idx;
        size_t pml3_idx;
        size_t pml2_idx;
        size_t pml1_idx;

        uint64_t *pml5_raw;
        uint64_t *pml4_raw;
        uint64_t *pml3_raw;
        uint64_t *pml2_raw;
        uint64_t *pml1_raw;
    };
};

inline uint64_t get_pml4() {
    uint64_t pml4;
    asm volatile ("mov %%cr3, %0" : "=r"(pml4));
    return pml4;
}

inline void tlb_flush() {
    asm volatile ("mov %0, %%cr3" :: "r" (get_pml4()) : "memory");
}

inline void invplg(uint64_t vaddr) {
    asm volatile ("invplg %0" :: "m"(vaddr) : "memory");
}

void init();

inline pmlx_table *kernel_mapping;

}

#endif
