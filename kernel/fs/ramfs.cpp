#include <fs/ramfs.hpp>
#include <vector.hpp>

namespace ramfs {

static lib::vector<node> node_list;

int vfs2ramfs(vfs::node *vfs_node, node *ret) {
    for(size_t i = 0; i < node_list.size(); i++) {
        if(node_list[i].vfs_node->absolute_path == vfs_node->absolute_path) {
            *ret = node_list[i];
            return 0;
        }
    }
    return -1;
}

int node::read(off_t off, off_t cnt, void *read_buf) {
    if(off > vfs_node->stat_cur->st_size)
        return -1;

    if((off + cnt) > vfs_node->stat_cur->st_size)
        cnt = vfs_node->stat_cur->st_size - off;

    memcpy8(buf + off, reinterpret_cast<uint8_t*>(read_buf), cnt);

    return cnt;
}

int node::write(off_t off, off_t cnt, void *write_buf) {
    if(buf == NULL) {
        vfs_node->stat_cur->st_size = off + cnt;
        buf = new uint8_t[off + cnt];
    }

    if((off + cnt) > vfs_node->stat_cur->st_size) {
        vfs_node->stat_cur->st_size += off + cnt - vfs_node->stat_cur->st_size;
        buf = reinterpret_cast<uint8_t*>(kmm::realloc(buf, vfs_node->stat_cur->st_size));
    }

    memcpy8(buf + off, reinterpret_cast<uint8_t*>(write_buf), cnt);

    return cnt;
}

node::~node() {
    for(size_t i = 0; i < node_list.size(); i++) 
        if(node_list[i].vfs_node->absolute_path == vfs_node->absolute_path)
            node_list.remove(i);
}

node::node(vfs::node *vfs_node) : vfs_node(vfs_node) {
    node_list.push(*this);
}

int fs::read(vfs::node *vfs_node, off_t off, off_t cnt, void *buf) {
    node *ramfs_node = new node;
    if(vfs2ramfs(vfs_node, ramfs_node) == -1)
        return -1;

    return ramfs_node->read(off, cnt, buf);
}

int fs::write(vfs::node *vfs_node, off_t off, off_t cnt, void *buf) {
    node *ramfs_node = new node; 
    if(vfs2ramfs(vfs_node, ramfs_node) == -1)
        return -1;

    return ramfs_node->write(off, cnt, buf);
}

int fs::open(vfs::node *vfs_node, uint16_t flags) {
    if(flags & fd::o_creat) {
        node new_node(vfs_node);
        return 0;
    }

    node *ramfs_node = new node;
    if(vfs2ramfs(vfs_node, ramfs_node) == 0)
        return 0;

    return -1;
}

int fs::mkdir([[maybe_unused]] vfs::node *vfs_node) {
    return 0;
}

int fs::unlink(vfs::node *vfs_node) {
    node *ramfs_node = new node;  
    if(vfs2ramfs(vfs_node, ramfs_node) == -1)
        return -1;

    ramfs_node->~node();

    return 0;
}

}
