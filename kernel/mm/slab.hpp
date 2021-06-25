#ifndef SLAB_HPP_
#define SLAB_HPP_

#include <mm/pmm.hpp>

namespace kmm {

constexpr size_t objects_per_slab = 256;
constexpr size_t largest_cache_size = 32768;

struct cache;
struct slab;

struct cache {
    cache(const char *name, size_t object_size, size_t flags);
    cache() = default;

    void *alloc_obj();
    int free_obj(void *obj);

    int move_slab(slab **dest_head, slab **src_head, slab *src);

    const char *name;
    size_t flags;

    size_t object_size;
    size_t active_objects;
    size_t active_slabs;
    size_t pages_per_slab;

    cache *next;
    cache *last;

    slab *slab_empty;
    slab *slab_partial; 
    slab *slab_full;
};

struct slab {
    void *alloc();
    void free(size_t index);

    size_t available_objects;
    cache *parent;

    uint8_t *buf;
    uint8_t *bitmap;

    slab *next;
    slab *last;
};

void *alloc(size_t cnt);
void *calloc(size_t cnt);

size_t free(void *obj); 

void *realloc(void *addr, size_t cnt);
void *recalloc(void *addr, size_t cnt);

}

inline void *operator new(size_t size) { return kmm::alloc(size); }
inline void *operator new[](size_t size) { return kmm::alloc(size); }

inline void *operator new([[maybe_unused]] size_t n, void *ptr) { return ptr; };

inline void operator delete(void *obj) { kmm::free(obj); }
inline void operator delete(void *obj, size_t) { kmm::free(obj); }

#endif
