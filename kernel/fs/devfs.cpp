#include <fs/devfs.hpp>
#include <fs/ext2/ext2.hpp>

namespace dev {

ssize_t node::read(size_t start, size_t cnt, void *ret) {
    if(!device)
        return -1;

    return device->read(start + partition_offset, cnt, ret);
}

ssize_t node::write(size_t start, size_t cnt, void *ret) {
    if(!device)
        return -1;

    return device->write(start + partition_offset, cnt, ret);
}

node::node(vfs::node *vfs_search_node) : vfs_node(NULL) {
    for(size_t i = 0; i < node_list.size(); i++) {
        if(vfs_search_node == node_list[i].vfs_node) {
            *this = node_list[i];
            return;
        }
    }
}

void scan_partitions(msd *device) {
    uint16_t mbr_signature;
    device->read(510, 2, &mbr_signature);

    if(mbr_signature == 0xaa55) {
        mbr_partition partitions[4];
        device->read(0x1be, sizeof(partitions), &partitions);

        for(size_t i = 0; i < 4; i++) {
            if(partitions[i].partition_type == 0)
                continue;

            lib::string absolute_path = lib::string("/dev/") + device->device_prefix + device->device_index + "-" + device->partition_cnt++;
            vfs::node new_vfs_node(absolute_path, NULL);

            print("[DEVFS] Creating partition device {}\n", absolute_path);

            node_list.push(node(vfs::root_node.search_absolute(absolute_path), partitions[i].starting_lba * device->sector_size, partitions[i].sector_cnt, device));

            ext2::fs(node_list.last());
        }

        return;
    }

    gpt_partition_table_hdr gpt_hdr;
    device->read(device->sector_size, sizeof(gpt_partition_table_hdr), &gpt_hdr);

    if(gpt_hdr.identifier == 0x4546492050415254) {
        size_t partition_array_lba = gpt_hdr.partition_array_lba;
        uint32_t entry_cnt = gpt_hdr.partition_ent_cnt;

        gpt_partition_entry *part_arr = new gpt_partition_entry[entry_cnt];

        device->read(partition_array_lba * device->sector_size, sizeof(gpt_partition_entry) * entry_cnt, part_arr);

        for(size_t i = 0; i < entry_cnt; i++) {
            if(part_arr[i].partition_type_guid[0] == 0 && part_arr[i].partition_type_guid[1] == 0) // unused entry
                continue;

            lib::string absolute_path = lib::string("/dev/") + device->device_prefix + device->device_index + "-" + device->partition_cnt++;
            vfs::node new_vfs_node(absolute_path, NULL);

            print("[DEVFS] Creating partition device {}\n", absolute_path);

            node_list.push(node(vfs::root_node.search_absolute(absolute_path), part_arr[i].starting_lba * device->sector_size, part_arr[i].starting_lba - part_arr[i].last_lba, device));
        }

        return;
    }
}

}
