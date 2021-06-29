#include <fs/fd.hpp>
#include <types.hpp>
#include <string.hpp>

namespace fs {

static size_t fd_cnt = 0;

fd::fd(lib::string path, int flags) : status(0) {
    auto open = [&, this]() {
        vfs_node = vfs::root_node.search_absolute(path);
        if(vfs_node == NULL && flags & o_creat) {
            vfs::node(path, NULL);
            vfs_node = vfs::root_node.search_absolute(path);
            if(vfs_node == NULL) {
                set_errno(enoent);
                return -1;
            }
            int ret = vfs_node->filesystem->open(vfs_node, flags);
            if(ret == -1) {
                vfs::root_node.remove(path);
            } 
            return ret;
        } else if(vfs_node == NULL) {
            set_errno(enoent);
            return -1; 
        }

        int ret = vfs_node->filesystem->open(vfs_node, flags);
        if(ret == -1)
            vfs::root_node.remove(path);

        return ret;
    } ();

    if(open == -1)
        return;

    _loc = (size_t*)kmm::calloc(sizeof(size_t)); 
    _flags = (size_t*)kmm::calloc(sizeof(size_t));

    status = 1;

    fd_list[fd_cnt++] = *this;
}

int fd::read(void *buf, size_t cnt) {
    if(vfs_node == NULL) {
        set_errno(enoent);
        return -1;
    }

    int ret = vfs_node->filesystem->read(vfs_node, *_loc, cnt, buf); 
    if(ret == -1)
        return -1;

    *_loc += cnt; 

    return ret;
}

int fd::write(void *buf, size_t cnt) {
    if(vfs_node == NULL) {
        set_errno(enoent);
        return -1;
    }

    int ret = vfs_node->filesystem->write(vfs_node, *_loc, cnt, buf); 
    if(ret == -1)
        return -1;

    *_loc += cnt; 

    return ret;
}

int fd::seek(off_t off, int whence) {
    if(vfs_node == NULL) {
        set_errno(enoent);
        return -1;
    }

    switch(whence) {
        case seek_set:
            return (*_loc = off);
        case seek_cur:
            return (*_loc += off);
        case seek_end:
            return (*_loc = vfs_node->stat_cur->st_size + off);
        default:
            set_errno(einval);
            return -1;
    }
}

}
