#ifndef EXT2_HPP_
#define EXT2_HPP_

#include <fs/devfs.hpp>
#include <utility>

namespace ext2 {

struct [[gnu::packed]] superblock { 
    uint32_t inode_cnt;
    uint32_t block_cnt;
    uint32_t sb_reserved;
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t sb_block;
    uint32_t block_size;
    uint32_t frag_size;
    uint32_t blocks_per_group; 
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mnt_time;
    uint32_t last_written_time;
    uint16_t mnt_cnt;
    uint16_t mnt_allowed;
    uint16_t signature; 
    uint16_t fs_state;
    uint16_t error_response;
    uint16_t version_min;
    uint32_t last_fsck;
    uint32_t forced_fsck;
    uint32_t os_id;
    uint32_t version_maj; 
    uint16_t user_id;
    uint16_t group_id;

    uint32_t first_inode;
    uint16_t inode_size;
    uint16_t sb_bgd;
    uint32_t opt_features;
    uint32_t req_features; 
    uint32_t non_supported_features;
    uint64_t uuid[2];
    uint64_t volume_name[2];
    uint64_t last_mnt_path[8];
};

class fs;

struct bgd {
    bgd(fs *parent, uint32_t bgd_index);
    bgd(bgd &buf);
    bgd() : parent(NULL) { }

    ssize_t find_block();
    void write_back();

    ssize_t alloc_block();
    ssize_t alloc_inode();

    bgd &operator=(bgd other) {
        swap(*this, other);
        return *this;
    }

    void swap(bgd &a, bgd &b) {
        std::swap(a.raw, b.raw);
        std::swap(a.parent, b.parent);
        std::swap(a.bgd_index, b.bgd_index);
    }

    struct [[gnu::packed]] {
        uint32_t block_addr_bitmap;
        uint32_t block_addr_inode;
        uint32_t inode_table_block;
        uint16_t unallocated_blocks;
        uint16_t unallocated_inodes;
        uint16_t dir_cnt;
        uint16_t reserved[7];
    } raw;

    fs *parent;
    uint32_t bgd_index;
};

struct inode {
    inode(fs *parent, uint32_t inode_index);
    inode(inode &buf);
    inode() = default;

    void read(off_t off, off_t cnt, void *buf);
    void write(off_t, off_t cnt, void *buf);

    ssize_t resize(off_t start, off_t cnt);
    void write_back();
    void remove();
    void free();

    ssize_t get_block(uint32_t iblock);
    ssize_t set_block(uint32_t iblock, uint32_t disk_block);

    inode &operator= (inode other) {
        swap(*this, other);
        return *this;
    }

    void swap(inode &a, inode &b) {
        std::swap(a.raw, b.raw);
        std::swap(a.parent, b.parent);
        std::swap(a.inode_index, b.inode_index);
    }

    struct [[gnu::packed]] {
        uint16_t permissions;
        uint16_t user_id;
        uint32_t size32l;
        uint32_t access_time;
        uint32_t creation_time;
        uint32_t mod_time;
        uint32_t del_time;
        uint16_t group_id;
        uint16_t hard_link_cnt;
        uint32_t sector_cnt;
        uint32_t flags;
        uint32_t oss1;
        uint32_t blocks[15];
        uint32_t gen_num;
        uint32_t eab;
        uint32_t size32h;
        uint32_t frag_addr;
    } raw;

    fs *parent;
    uint32_t inode_index;
};

struct dir {
    dir(inode *parent_inode, lib::string path);

    struct raw_dir {
        uint32_t inode;
        uint16_t entry_size;
        uint8_t name_length;
        uint8_t type;
        char name[];
    };

    raw_dir *raw;

    inode *parent_inode;
    lib::string path;
private:
    ssize_t search_relative(lib::string path);
};

class fs {
public:
    fs(dev::node &devfs_node);
    fs() = default;

    int open(vfs::node *vfs_node, int flags);
    int read(vfs::node *vfs_node, off_t off, off_t cnt, void *buf);
    int write(vfs::node *vfs_node, off_t off, off_t cnt, void *buf);
    int refresh(vfs::node *vfs_node);
    int unlink(vfs::node *vfs_node);

    friend class bgd;
    friend class inode;
private:
    superblock superb;
    inode root_inode;

    size_t block_size;
    size_t frag_size;
    uint32_t bgd_cnt;

    dev::node devfs_node;

    ssize_t alloc_block();
    void free_block(uint32_t block);

    ssize_t alloc_inode();
    void free_inode(uint32_t inode_index);

    void delete_inode(inode &inode_cur);
};

}

#endif
