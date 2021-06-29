#include <fs/ext2/ext2.hpp>

namespace ext2 {

inode::inode(fs *parent, uint32_t inode_index) : parent(parent), inode_index(inode_index) {
    uint32_t inode_table_index = (inode_index - 1) % parent->superb.inodes_per_group;
    uint32_t bgd_index = (inode_index - 1) / parent->superb.inodes_per_group;

    bgd inode_bgd(parent, bgd_index);

    parent->devfs_node.read(inode_bgd.raw.inode_table_block * parent->block_size + parent->superb.inode_size * inode_table_index, sizeof(raw), reinterpret_cast<void*>(&raw));
}

inode::inode(inode &buf) {
    raw = buf.raw;
    parent = buf.parent;
    inode_index = buf.inode_index;
    write_back();
}

void inode::write_back() {
    uint32_t inode_table_index = (inode_index - 1) % parent->superb.inodes_per_group;
    uint32_t bgd_index = (inode_index - 1) / parent->superb.inodes_per_group;

    bgd inode_bgd(parent, bgd_index);

    parent->devfs_node.write(inode_bgd.raw.inode_table_block * parent->block_size + parent->superb.inode_size * inode_table_index, sizeof(raw), reinterpret_cast<void*>(&raw));
}

void inode::read(off_t off, off_t cnt, void *buf) {
    off_t headway = 0;

    while(headway < cnt) {
        uint32_t iblock = (off + headway) / parent->block_size;

        size_t size = cnt - headway;
        size_t offset = (off + headway) % parent->block_size;

        if(size > parent->block_size - offset)
            size = parent->block_size - offset;

        uint32_t disk_block = get_block(iblock);

        parent->devfs_node.read(disk_block * parent->block_size + offset, size, reinterpret_cast<void*>(reinterpret_cast<size_t>(buf) + headway));

        headway += size;
    }
} 

void inode::write(off_t off, off_t cnt, void *buf) {
    off_t headway = 0;

    resize(off, cnt);

    while(headway < cnt) {
        uint32_t iblock = (off + headway) / parent->block_size;

        size_t size = cnt - headway;
        size_t offset = (off + headway) % parent->block_size;

        if(size > parent->block_size - offset)
            size = parent->block_size - offset;

        uint32_t disk_block = get_block(iblock);

        parent->devfs_node.write(disk_block * parent->block_size + offset, size, reinterpret_cast<void*>(reinterpret_cast<size_t>(buf) + headway));

        headway += size;
    }
}

ssize_t inode::resize(off_t start, off_t cnt) {
    size_t sector_size = parent->devfs_node.device->sector_size;

    if((start + cnt) < (signed)(raw.sector_cnt * sector_size))
        return 0;

    uint32_t iblock_start = div_roundup(raw.sector_cnt * sector_size, parent->block_size);
    uint32_t iblock_end = div_roundup(start + cnt, parent->block_size);

    if(raw.size32l < (start + cnt)) {
        raw.size32l = start + cnt;
    }

    for(size_t i = iblock_start; i < iblock_end; i++) {
        ssize_t disk_block = parent->alloc_block();
        if(disk_block == -1)
            return -1;

        raw.sector_cnt += parent->block_size / sector_size;
        set_block(i, disk_block);
    }

    write_back(); 

    return 0;
}

ssize_t inode::get_block(uint32_t iblock) {
    ssize_t block_index = -1;
    ssize_t blocks_per_level = parent->block_size / 4;

    if(iblock < 12) {
        return raw.blocks[iblock];
    } else {
        iblock -= 12;

        if(iblock >= blocks_per_level) {
            iblock -= blocks_per_level;

            uint32_t single_index = iblock / blocks_per_level;
            uint32_t indirect_offset = iblock % blocks_per_level;
            uint32_t indirect_block;

            if(single_index >= blocks_per_level) { // triple indirect
                iblock -= blocks_per_level * blocks_per_level; 
                uint32_t double_indirect_index = iblock / blocks_per_level;
                uint32_t single_indirect_index;

                parent->devfs_node.read(raw.blocks[14] * parent->block_size + double_indirect_index * 4, 4, &single_indirect_index);
                parent->devfs_node.read(double_indirect_index * parent->block_size + single_indirect_index * 4, 4, &indirect_block);
                parent->devfs_node.read(indirect_block * parent->block_size + indirect_offset * 4, 4, &block_index);
            } else { // double indirect
                parent->devfs_node.read(raw.blocks[13] * parent->block_size + single_index * 4, 4, &indirect_block);
                parent->devfs_node.read(indirect_block * parent->block_size + indirect_offset * 4, 4, &block_index);
            }
        } else { // singly indirect
            parent->devfs_node.read(raw.blocks[12] * parent->block_size + iblock * 4, 4, &block_index);
        }
    }

    return block_index;
}

ssize_t inode::set_block(uint32_t iblock, uint32_t disk_block) {
    ssize_t blocks_per_level = parent->block_size / 4;

    if(iblock < 12) {
        raw.blocks[iblock] = disk_block;
    } else {
        iblock -= 12;
        
        if(iblock >= blocks_per_level) {
            iblock -= blocks_per_level;

            uint32_t single_index = iblock / blocks_per_level;
            uint32_t indirect_offset = iblock % blocks_per_level;
            uint32_t indirect_block;

            if(single_index >= blocks_per_level) { // triple indirect
                iblock -= blocks_per_level * blocks_per_level; 
                uint32_t double_indirect_index = iblock / blocks_per_level;
                uint32_t single_indirect_index;

                if(!raw.blocks[14]) {
                    ssize_t block = parent->alloc_block();
                    if(block == -1)
                        return -1;
                    raw.blocks[14] = block;
                    write_back();
                }

                parent->devfs_node.read(raw.blocks[14] * parent->block_size + double_indirect_index * 4, 4, &single_indirect_index);
                if(!single_indirect_index) {
                    ssize_t block = parent->alloc_block();
                    if(block == -1)
                        return -1;
                    single_indirect_index = block;
                    parent->devfs_node.write(raw.blocks[14] * parent->block_size + double_indirect_index * 4, 4, &single_indirect_index);
                }

                parent->devfs_node.read(double_indirect_index * parent->block_size + single_indirect_index * 4, 4, &indirect_block);
                if(!indirect_block) {
                    ssize_t block = parent->alloc_block();
                    if(block == -1)
                        return -1;
                    indirect_block = block;
                    parent->devfs_node.write(double_indirect_index * parent->block_size + single_indirect_index * 4, 4, &indirect_block);
                }

                parent->devfs_node.write(indirect_block * parent->block_size + indirect_offset * 4, 4, &disk_block);
            } else { // double indriect
                if(!raw.blocks[13]) {
                    ssize_t block = parent->alloc_block();
                    if(block == -1)
                        return -1;
                    raw.blocks[13] = block;
                    write_back();
                }

                parent->devfs_node.read(raw.blocks[13] * parent->block_size + single_index * 4, 4, &indirect_block);
                if(!indirect_block) {
                    ssize_t block = parent->alloc_block();
                    if(block == -1)
                        return -1;
                    indirect_block = block;
                    parent->devfs_node.write(raw.blocks[13] * parent->block_size + single_index * 4, 4, &indirect_block);
                }

                parent->devfs_node.write(indirect_block * parent->block_size + indirect_offset * 4, 4, &disk_block);
            }
        } else {
            if(!raw.blocks[12]) {
                ssize_t block = parent->alloc_block();
                if(block == -1)
                    return -1;
                raw.blocks[12] = block;
                write_back();
            }
        }
    }
    return disk_block;
}

void inode::remove() {
    for(ssize_t i = 0; i < div_roundup(raw.sector_cnt * parent->devfs_node.device->sector_size, parent->block_size); i++) {
        parent->free_block(get_block(i));
    }

    parent->free_inode(inode_index);
}

}
