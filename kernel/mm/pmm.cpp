#include <mm/pmm.hpp>
#include <debug.hpp>

namespace pmm {

class mem_chunk {
public:
    mem_chunk(size_t base, size_t page_cnt);
    mem_chunk() = default;

    mem_chunk *append_chunk(mem_chunk &&chunk);

    size_t alloc(size_t cnt, size_t align);
    void free(size_t base, size_t cnt);

    size_t base;
    size_t page_cnt;

    mem_chunk *next;
    mem_chunk *last;

    static size_t buffer;
    static void *chunk_alloc(size_t cnt);
private:
    size_t bitmap_size;
    size_t bitmap_cnt;
    uint8_t *bitmap;
};

static mem_chunk *root = NULL;
static size_t pmm_lock = 0;

size_t mem_chunk::buffer = 0;

void init(stivale *stivale) {
    stivale_mmap *mmap = reinterpret_cast<stivale_mmap*>(stivale->mmap_addr + vmm::high_vma);

    auto max_size = [&]() {
        size_t buffer_size = 0;
        for(size_t i = 0; i < stivale->mmap_cnt; i++) {
            if(mmap[i].type == 1)
                buffer_size += sizeof(mem_chunk) + (mmap[i].len / vmm::page_size / 8);
            total_mem += mmap[i].len;
        }

        return buffer_size; 
    }();

    for(size_t i = 0; i < stivale->mmap_cnt; i++) {
        print("[{x}] -> [{x}] length {x}\n", mmap[i].addr, mmap[i].addr + mmap[i].len, mmap[i].len);
    }

    print("Total memory: {x}\n", total_mem);

    size_t i;
    for(i = 0; i < stivale->mmap_cnt; i++) {
        if(mmap[i].type == 1 && mmap[i].len >= max_size) {
            mem_chunk::buffer = mmap[i].addr + vmm::high_vma;
            mmap[i].addr += max_size;
            mmap[i].len -= max_size;
            break; 
        }
    }

    for(; i < stivale->mmap_cnt; i++) {
        if(mmap[i].type == 1 && mmap[i].len) {
            size_t save = mmap[i].addr;
            mmap[i].addr = align_up(mmap[i].addr, vmm::page_size);
            mmap[i].len -= mmap[i].addr - save;

            if(root) {
                root->append_chunk(mem_chunk(mmap[i].addr, mmap[i].len / vmm::page_size));
            } else {
                root = (mem_chunk*)mem_chunk::chunk_alloc(sizeof(mem_chunk));
                *root = mem_chunk(mmap[i].addr, mmap[i].len / vmm::page_size);
            }
        }
    }
}

mem_chunk *mem_chunk::append_chunk(mem_chunk &&chunk) {
    mem_chunk *node = this;
    while(node->next != NULL)
        node = node->next;

    node->next = reinterpret_cast<mem_chunk*>(buffer);
    buffer += sizeof(mem_chunk);
    *node->next = chunk;

    return node->next;
}

mem_chunk::mem_chunk(size_t base, size_t page_cnt) : base(base), page_cnt(page_cnt) {
    bitmap_size = div_roundup(page_cnt, 8);
    bitmap = (uint8_t*)chunk_alloc(bitmap_size);
    memset8(bitmap, 0, bitmap_size);
}

size_t mem_chunk::alloc(size_t cnt, size_t align) {
    spin_lock(&pmm_lock);

    size_t alloc_base = align_up(base, align * vmm::page_size);
    size_t bit_base = (alloc_base - base) / vmm::page_size;

    for(size_t i = bit_base; i < page_cnt; i += align) {
        for(size_t j = i, count = 0; j < i + cnt; j++) {
            if(bm_test(bitmap, j)) {
                alloc_base += align * vmm::page_size;
                break;
            }

            if(++count == cnt) {
                for(size_t z = 0; z < count; z++)
                    bm_set(bitmap, i + z);
                spin_release(&pmm_lock);
                return alloc_base;
            }
        }
    }

    spin_release(&pmm_lock);
    return -1;
}

void mem_chunk::free(size_t base, size_t cnt) {
    spin_lock(&pmm_lock);
    for(size_t i = div_roundup(base, vmm::page_size); i < div_roundup(base, vmm::page_size) + cnt; i++) {
        bm_clear(bitmap, i);
    }
    spin_release(&pmm_lock);
}

void *mem_chunk::chunk_alloc(size_t cnt) {
    void *ret = reinterpret_cast<void*>(buffer);
    buffer += cnt;
    return ret;
}

size_t alloc(size_t cnt, size_t align) {
    mem_chunk *chunk = root;
    do {
        size_t alloc = chunk->alloc(cnt, align);
        if(alloc == -1ull)
            chunk = chunk->next;
        else
            return alloc;
    } while(chunk != NULL);

    print("PMM: out of memory\n");

    return -1;
}

size_t calloc(size_t cnt, size_t align) {
    size_t allocation = alloc(cnt, align);
    memset64((uint64_t*)(allocation + vmm::high_vma), 0, (cnt * vmm::page_size) / 8);
    return allocation;
}

void free(size_t base, size_t cnt) {
    mem_chunk *chunk = root;
    do {
        if(base >= chunk->base && (base + cnt * vmm::page_size) <= (chunk->base + chunk->page_cnt * vmm::page_size))
            return chunk->free(base - chunk->base, cnt);
        chunk = chunk->next;
    } while(chunk != NULL);
}

}
