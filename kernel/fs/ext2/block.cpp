#include <fs/ext2/ext2.hpp>

namespace ext2 {

bgd::bgd(fs *parent, uint32_t bgd_index) : parent(parent), bgd_index(bgd_index) {
    size_t bgd_offset = parent->block_size >= 2048 ? parent->block_size : parent->block_size * 2;
    size_t group_index = (bgd_index - 1) / parent->superb.inodes_per_group;

    parent->devfs_node.read(bgd_offset + sizeof(raw) * group_index, sizeof(raw), reinterpret_cast<void*>(&raw));
} 

bgd::bgd(bgd &buf) {
    *this = buf;
    write_back();
}

void bgd::write_back() {
    size_t bgd_offset = parent->block_size >= 2048 ? parent->block_size : parent->block_size * 2;
    size_t group_index = (bgd_index - 1) / parent->superb.inodes_per_group;

    parent->devfs_node.write(bgd_offset + sizeof(raw) * group_index, sizeof(raw), reinterpret_cast<void*>(&raw));
}

ssize_t bgd::alloc_block() {
    if(parent == NULL)
        return -1;

    if(!raw.unallocated_blocks)
        return -1;

    uint8_t *bitmap = new uint8_t[parent->block_size];

    parent->devfs_node.read(raw.block_addr_bitmap, parent->block_size, reinterpret_cast<void*>(bitmap));

    for(size_t i = 0; i < parent->block_size; i++) {
        if(!bm_test(bitmap, i)) {
            bm_set(bitmap, i);
            parent->devfs_node.write(raw.block_addr_bitmap, parent->block_size, reinterpret_cast<void*>(bitmap));

            raw.unallocated_blocks--;
            write_back();

            delete bitmap;
            return i;            
        }
    }

    delete bitmap;
    return -1;
}

ssize_t bgd::alloc_inode() {
    if(parent == NULL)
        return -1;

    if(!raw.unallocated_inodes)
        return -1;

    uint8_t *bitmap = new uint8_t[parent->block_size];

    parent->devfs_node.read(raw.block_addr_inode, parent->block_size, reinterpret_cast<void*>(bitmap));

    for(size_t i = 0; i < parent->block_size; i++) {
        if(!bm_test(bitmap, i)) {
            bm_set(bitmap, i);
            parent->devfs_node.write(raw.block_addr_inode, parent->block_size, reinterpret_cast<void*>(bitmap));

            raw.unallocated_inodes--;
            write_back();

            delete bitmap;
            return i;            
        }
    }

    delete bitmap;
    return -1;
}

}
