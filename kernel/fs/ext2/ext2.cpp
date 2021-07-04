#include <fs/ext2/ext2.hpp>

namespace ext2 {

fs::fs(dev::node &devfs_node) : devfs_node(devfs_node) {
    devfs_node.read(devfs_node.device->sector_size * 2, sizeof(superblock), &superb);

    if(superb.signature != 0xef53) {
        return;
    }

    print("[EXT2] Filesystem Detected on Device {}\n", devfs_node.vfs_node->absolute_path);
    print("[EXT2] Inode cnt: {}\n", (unsigned)superb.inode_cnt);
    print("[EXT2] Inodes per group: {}\n", (unsigned)superb.inodes_per_group);
    print("[EXT2] Block cnt: {}\n", (unsigned)superb.block_cnt);
    print("[EXT2] Blocks per group: {}\n", (unsigned)superb.blocks_per_group);
    print("[EXT2] First non-reserved inode: {}\n", (unsigned)superb.first_inode);

    block_size = 1024 << superb.block_size;
    frag_size = 1024 << superb.frag_size;
    bgd_cnt = div_roundup(superb.block_cnt, superb.blocks_per_group);

    root_inode = inode(this, 2);

    devfs_node.filesystem = this;
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

int fs::read(vfs::node *vfs_node, off_t off, off_t cnt, void *buf) {
    if(off > vfs_node->stat_cur->st_size) {
        return -1;
    }

    if((off + cnt) > vfs_node->stat_cur->st_size) {
        cnt = vfs_node->stat_cur->st_size - off;
    }

    dir file_dir_entry(&root_inode, vfs_node->relative_path, true);
    if(file_dir_entry.raw == NULL) 
        return -1;

    inode inode_cur(this, file_dir_entry.raw->inode);
    inode_cur.read(off, cnt, buf);

    return cnt;
}

int fs::write(vfs::node *vfs_node, off_t off, off_t cnt, void *buf) {
    if(off > vfs_node->stat_cur->st_size) {
        return -1;
    }

    dir file_dir_entry(&root_inode, vfs_node->relative_path, true);
    if(file_dir_entry.raw == NULL) 
        return -1;

    inode inode_cur(this, file_dir_entry.raw->inode);
    inode_cur.write(off, cnt, buf);

    vfs_node->stat_cur->st_size += cnt;

    return cnt;
}

int fs::refresh_node(inode &inode_cur, lib::string mount_gate) {
    uint8_t *buffer = new uint8_t[inode_cur.raw.size32l];
    inode_cur.read(0, inode_cur.raw.size32l, buffer);

    for(uint32_t i = 0; i < inode_cur.raw.size32l;) { 
        raw_dir *dir_cur = reinterpret_cast<raw_dir*>(buffer + i);
        
        lib::string name(reinterpret_cast<char*>(dir_cur->name), dir_cur->name_length);
        lib::string absolute_path = mount_gate + name;

        if(name == "." || name == "..") {
            vfs::node(absolute_path, NULL); 

            uint32_t expected_size = align_up(sizeof(raw_dir) + dir_cur->name_length, 4);
            if(dir_cur->entry_size != expected_size) {
                delete buffer;
                return -1;
            }

            i += dir_cur->entry_size;

            continue;
        }

        inode new_inode(this, dir_cur->inode);

        if(new_inode.raw.permissions & 0x4000) { // is a directory
            lib::string directory_path = absolute_path + "/";
            vfs::node(directory_path, NULL); 
            refresh_node(new_inode, directory_path);
        } else {
            vfs::node(absolute_path, NULL); 
        }

        uint32_t expected_size = align_up(sizeof(raw_dir) + dir_cur->name_length, 4);
        if(dir_cur->entry_size != expected_size) {
            delete buffer;
            return -1;
        }

        i += dir_cur->entry_size;
    }

    delete buffer;

    return 0;
}

int fs::unlink(vfs::node *vfs_node) {
    dir delete_dir(&root_inode, vfs_node->relative_path, false);
    if(delete_dir.exists == false) 
        return -1;
    return 0; 
}

int fs::refresh(vfs::node *vfs_node) {
    return refresh_node(root_inode, vfs_node->absolute_path);
}

int fs::open(vfs::node *vfs_node, uint16_t flags) {
    if(flags & o_creat) {
        inode parent_inode;
        
        if(vfs_node->parent->relative_path == "/") {
            parent_inode = root_inode;
        } else {
            dir dir_entry(&root_inode, vfs_node->parent->relative_path, true); 
            if(dir_entry.raw == NULL) {
                return -1;
            }

            parent_inode = inode(this, dir_entry.raw->inode);
        }

        inode new_inode(this, alloc_inode());

        if(new_inode.set_block(0, alloc_block()) == -1)
            return -1;

        new_inode.raw.sector_cnt = block_size / devfs_node.device->sector_size;
        new_inode.write_back();

        char *path = [&]() {
            char *ret = vfs_node->absolute_path.data();
            ssize_t offset = vfs_node->absolute_path.find_last('/');
            return ret + offset + 1;
        } ();

        dir create_dir_entry(&parent_inode, new_inode.inode_index, 0, path);
        parent_inode.raw.hard_link_cnt++;
        parent_inode.write_back();

        return 0;
    }

    dir dir_entry(&root_inode, vfs_node->relative_path, true); 
    if(dir_entry.raw == NULL)
        return -1;

    inode inode_cur(this, dir_entry.raw->inode);
    vfs_node->stat_cur->st_size = inode_cur.raw.size32l;

    return 0;
}

}
