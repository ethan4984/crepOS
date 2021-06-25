#include <fs/ext2/ext2.hpp>

namespace ext2 {

fs::fs(dev::node &devfs_node) : devfs_node(devfs_node) {
    devfs_node.read(devfs_node.device->sector_size * 2, sizeof(superblock), &superb);

    if(superb.signature != 0xef53) {
        return;
    }

    print("[EXT2] Filesystem Detected on Device {}\n", devfs_node.vfs_node->absolute_path);

    block_size = 1024 << superb.block_size;
    frag_size = 1024 << superb.frag_size;
    bgd_cnt = superb.block_cnt / superb.blocks_per_group;

    root_inode = inode(this, 2);
}

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

inode::inode(fs *parent, uint32_t inode_index) : parent(parent), inode_index(inode_index) {
    uint32_t inode_table_index = (inode_index - 1) % parent->superb.inodes_per_group;

    bgd inode_bgd(parent, inode_index);

    parent->devfs_node.read(inode_bgd.raw.inode_table_block * parent->block_size + parent->superb.inode_size * inode_table_index, sizeof(raw), reinterpret_cast<void*>(&raw));
}

inode::inode(inode &buf) {
    *this = buf;
    write_back();
} 

void inode::write_back() {
    uint32_t inode_table_index = (inode_index - 1) % parent->superb.inodes_per_group;

    bgd inode_bgd(parent, inode_index);

    parent->devfs_node.write(inode_bgd.raw.inode_table_block * parent->block_size + parent->superb.inode_size * inode_table_index, sizeof(raw), reinterpret_cast<void*>(&raw));
}

ssize_t fs::alloc_block() {
    for(size_t i = 0; i < bgd_cnt; i++) {
        bgd bgd_cur(this, i);
        ssize_t block = bgd_cur.alloc_block();
        if(block == -1) {
            continue;
        } else {
            return i * superb.blocks_per_group + block;
        }
    }
    return -1;
}

void fs::free_block(uint32_t block) {
    uint8_t *bitmap = new uint8_t[block_size];
    uint32_t bgd_index = block / superb.blocks_per_group;
    uint32_t bitmap_index = block - bgd_index * superb.blocks_per_group;

    bgd bgd_cur(this, bgd_index);

    devfs_node.read(bgd_cur.raw.block_addr_bitmap, block_size, reinterpret_cast<void*>(bitmap));
    if(!bm_test(bitmap, bitmap_index)) {
        delete bitmap;
        return;
    }

    bm_clear(bitmap, bitmap_index);
    devfs_node.write(bgd_cur.raw.block_addr_bitmap, block_size, reinterpret_cast<void*>(bitmap));

    bgd_cur.raw.unallocated_blocks++;
    bgd_cur.write_back();

    delete bitmap;
}

ssize_t fs::alloc_inode() {
    for(size_t i = 0; i < bgd_cnt; i++) {
        bgd bgd_cur(this, i);
        ssize_t inode_index = bgd_cur.alloc_inode();
        if(inode_index == -1) {
            continue;
        } else {
            return i * superb.inodes_per_group + inode_index;
        }
    }
    return -1;
}

void fs::free_inode(uint32_t inode_index) {
    uint8_t *bitmap = new uint8_t[block_size];
    uint32_t bgd_index = inode_index / superb.inodes_per_group;
    uint32_t bitmap_index = inode_index - bgd_index * superb.inodes_per_group;

    bgd bgd_cur(this, bgd_index);

    devfs_node.read(bgd_cur.raw.block_addr_inode, block_size, reinterpret_cast<void*>(bitmap));
    if(!bm_test(bitmap, bitmap_index)) {
        delete bitmap;
        return;
    }

    bm_clear(bitmap, bitmap_index);
    devfs_node.write(bgd_cur.raw.block_addr_inode, block_size, reinterpret_cast<void*>(bitmap));

    bgd_cur.raw.unallocated_inodes++;
    bgd_cur.write_back();

    delete bitmap;
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

void fs::delete_inode(inode &inode_cur) {
    for(ssize_t i = 0; i < div_roundup(inode_cur.raw.sector_cnt * devfs_node.device->sector_size, block_size); i++)
        free_block(inode_cur.get_block(i));

    free_inode(inode_cur.inode_index);
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

ssize_t dir::search_relative(lib::string path) {
    uint8_t *buffer = new uint8_t[parent_inode->raw.size32l];
    parent_inode->read(0, parent_inode->raw.size32l, buffer);

    for(uint32_t i = 0; i < parent_inode->raw.size32l;) { 
        raw_dir *dir_cur = reinterpret_cast<raw_dir*>(buffer + i);

        lib::string name(reinterpret_cast<char*>(dir_cur->name), dir_cur->name_length);

        if(path == name) {
            if(dir_cur->inode == 0) {
                delete buffer;
                return -1;
            }

            raw = dir_cur;

            return 0;
        }

        uint32_t expected_size = align_up(sizeof(raw_dir) + dir_cur->name_length, 4);
        if(dir_cur->entry_size != expected_size)
            break;

        i += dir_cur->entry_size;
    }

    delete buffer;
    return -1;
}

dir::dir(inode *parent_inode, lib::string path) : raw(NULL), parent_inode(parent_inode), path(path) {
    lib::vector<lib::string> sub_paths = [&]() {
        size_t start = 0;
        size_t end = path.find_first('/');
 
        lib::vector<lib::string> ret;
 
        while(end != lib::string::npos) {
            ret.push(path.substr(start, end - start));
            start = end + 1;
            end = path.find_first('/', start);
        }
 
        ret.push(path.substr(start, end));
 
        return ret;
    } ();

    for(size_t i = 0; i < sub_paths.size(); i++) {
        if(search_relative(sub_paths[i]) == -1) {
            return;
        }

        *parent_inode = inode(parent_inode->parent, raw->inode);
    }
}

}
