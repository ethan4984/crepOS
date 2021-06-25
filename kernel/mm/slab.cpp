#include <mm/slab.hpp>
#include <cpu.hpp>

kmm::cache cache32(NULL, 0, 32);

namespace kmm {

static cache *cache_root = NULL;
static size_t slab_lock = 0;

static slab *alloc_slab(cache *parent) { 
    slab *new_slab = reinterpret_cast<slab*>(pmm::calloc(parent->pages_per_slab) + vmm::high_vma);

    new_slab->bitmap = reinterpret_cast<uint8_t*>(reinterpret_cast<size_t>(new_slab) + sizeof(slab));
    new_slab->buf = reinterpret_cast<uint8_t*>(align_up(reinterpret_cast<size_t>(new_slab) + sizeof(slab) + objects_per_slab / 8, 16));
    new_slab->available_objects = objects_per_slab;
    new_slab->parent = parent;

    if(parent->slab_empty == NULL) { 
        parent->slab_empty = new_slab;
    } else {
        slab *node = parent->slab_empty;
        while(node->next != NULL)
            node = node->next;
        node->next = new_slab;
        new_slab->last = node;
    }

    return new_slab;
}

cache::cache(const char *name, size_t flags, size_t object_size) : name(name), flags(flags), object_size(object_size) {
    pages_per_slab = div_roundup(object_size * objects_per_slab, vmm::page_size);

    slab *root_slab = alloc_slab(this);

    *reinterpret_cast<cache*>(root_slab->buf) = *this;
    cache *new_cache = reinterpret_cast<cache*>(root_slab->buf);
    root_slab->buf += sizeof(cache);

    root_slab->parent = new_cache;

    cache *node = cache_root;
    if(node == NULL) {
        cache_root = new_cache;
    } else {
        while(node->next != NULL)
            node = node->next;
        node->next = new_cache;
        new_cache->last = node;
    }
}

void *cache::alloc_obj() {
    slab *slab_cur = NULL;

    if(slab_partial) {
        slab_cur = slab_partial;
    } else {
        slab_cur = slab_empty;
    }

    if(!slab_cur) {
        slab_cur = alloc_slab(this);
        slab_empty = slab_cur;
    }

    void *addr = slab_cur->alloc();

    if(!slab_cur->available_objects) {
        move_slab(&slab_full, &slab_partial, slab_cur);
    } else if(slab_cur->available_objects == objects_per_slab - 1) {
        move_slab(&slab_partial, &slab_empty, slab_cur); 
    }

    return addr;
}

int cache::free_obj(void *obj) {
    auto scan_slab = [this, &obj](slab *slab_cur) {
        if(!slab_cur)
            return -1;

        if(slab_cur->buf <= obj && (slab_cur->buf + object_size * objects_per_slab) > obj) {
            size_t index = (reinterpret_cast<size_t>(obj) - reinterpret_cast<size_t>(slab_cur->buf)) / object_size;
            bm_clear(slab_cur->bitmap, index);
            return 0;
        }

        return -1;
    };

    int ret = scan_slab(slab_partial);
    if(ret == 0)
        return ret;

    ret = scan_slab(slab_full);
    if(ret == 0)
        return ret;

    return -1;
}

int cache::move_slab(slab **dest_head, slab **src_head, slab *src) {
    if(!src || !*src_head)
        return -1; 

    if(src->next != NULL)
        src->next->last = src->last;
    if(src->last != NULL)
        src->last->next = src->next;
    if(*src_head == src)
        *src_head = src->next;

    if(!*dest_head) {
        src->last = NULL;
        src->next = NULL;
        *dest_head = src; 
        return 0;
    }

    slab *node = *dest_head;
    while(node->next != NULL)
        node = node->next;
    node->next = src;
    src->last = node;

    return 0;
}

void *slab::alloc() {
    for(size_t i = 0; i < objects_per_slab; i++) {
        if(!bm_test(bitmap, i)) {
            bm_set(bitmap, i); 
            available_objects--;
            return buf + (i * parent->object_size);
        }
    }

    return NULL;
}

void slab::free(size_t index) {
    if(bm_test(bitmap, index)) { 
        available_objects++;
        bm_clear(bitmap, index);
    }
}

void *alloc(size_t size) {
    if(!size)
        return NULL;

    size_t round_size = pow2_roundup(size);
    if(round_size <= 16)
        round_size = 32;

    cache *cache_cur = cache_root;

    spin_lock(&slab_lock);

    do {
        if(cache_cur->object_size == round_size) {
            spin_release(&slab_lock);
            return cache_cur->alloc_obj();
        }
        cache_cur = cache_cur->next;
    } while(cache_cur != NULL);

    spin_release(&slab_lock);

    return NULL;
}

size_t free(void *obj) {
    if(obj == NULL)
        return 0;

    spin_lock(&slab_lock);

    cache *cache_cur = cache_root;

    do {
        if(cache_cur->free_obj(obj)) {
            spin_release(&slab_lock);
            return cache_cur->object_size;
        }
        cache_cur = cache_cur->next;
    } while(cache_cur != NULL);

    spin_release(&slab_lock);

    return 0;
}

void *calloc(size_t cnt) {
    uint8_t *addr = reinterpret_cast<uint8_t*>(alloc(cnt));
    if(addr == NULL) {
        return NULL;
    }

    memset8(addr, 0, cnt);

    return addr; 
}

void *realloc(void *addr, size_t cnt) {
    if(addr == NULL) 
        return NULL;

    size_t alloc_size = free(addr);

    void *new_addr = alloc(cnt);
    
    size_t bytes_to_copy = cnt;
    if(cnt > alloc_size) 
        bytes_to_copy = alloc_size;

    memcpy8(reinterpret_cast<uint8_t*>(new_addr), reinterpret_cast<uint8_t*>(addr), bytes_to_copy);

    return new_addr;
}

void *recalloc(void *addr, size_t cnt) {
    if(addr == NULL) 
        return NULL;

    size_t alloc_size = free(addr);

    void *new_addr = calloc(cnt);
    
    size_t bytes_to_copy = cnt;
    if(cnt >= alloc_size) 
        bytes_to_copy = alloc_size;

    memcpy8(reinterpret_cast<uint8_t*>(new_addr), reinterpret_cast<uint8_t*>(addr), bytes_to_copy);

    return new_addr;
}

}
