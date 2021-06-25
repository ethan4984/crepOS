#ifndef DEVFS_HPP_
#define DEVFS_HPP_

#include <fs/vfs.hpp>

namespace dev {

struct msd {
    virtual ssize_t read([[maybe_unused]] size_t off, [[maybe_unused]] size_t cnt, [[maybe_unused]] void *buf) {
        print("[MSD] Unimplemented write to a mass storage device\n");
        return -1;
    }

    virtual ssize_t write([[maybe_unused]] size_t off, [[maybe_unused]] size_t cnt, [[maybe_unused]] void *buf) {
        print("[MSD] Unimplemented read to a mass storage device\n");
        return -1;
    }

    lib::string device_prefix;
    ssize_t device_index;
    ssize_t partition_index;
    ssize_t partition_cnt;
    ssize_t sector_size;
    ssize_t sector_cnt;
    size_t device_type;
};

class node {
public:
    node(vfs::node *vfs_node, size_t partition_offset, size_t sector_cnt, msd *device) : vfs_node(vfs_node), device(device), partition_offset(partition_offset), sector_cnt(sector_cnt) { }
    node(vfs::node *vfs_node, size_t sector_cnt, msd *device) : vfs_node(vfs_node), device(device), sector_cnt(sector_cnt) { }
    node(vfs::node *vfs_node);
    node() = default;
    
    ssize_t read(size_t start, size_t cnt, void *ret);
    ssize_t write(size_t start, size_t cnt, void *ret);

    vfs::node *vfs_node;
    msd *device;
private:
    size_t partition_offset;
    size_t sector_cnt;
};

struct [[gnu::packed]] mbr_partition {
    uint8_t drive_status;
    uint8_t starting_chs[3];
    uint8_t partition_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t sector_cnt;
};

struct [[gnu::packed]] gpt_partition_table_hdr {
    uint64_t identifier;
    uint32_t version;
    uint32_t hdr_size;
    uint32_t checksum;
    uint32_t reserved0;
    uint64_t hdr_lba;
    uint64_t alt_hdr_lba;
    uint64_t first_block;
    uint64_t last_block;
    uint64_t guid[2];
    uint64_t partition_array_lba;
    uint32_t partition_ent_cnt;
    uint32_t partition_ent_size;
    uint32_t crc32_partition_array;
};

struct [[gnu::packed]] gpt_partition_entry {
    uint64_t partition_type_guid[2];
    uint64_t partition_guid[2];
    uint64_t starting_lba;
    uint64_t last_lba;
    uint64_t flags;
    uint64_t name[9];
};

void scan_partitions(msd *device);
void init();

inline lib::vector<node> node_list;

}

#endif
