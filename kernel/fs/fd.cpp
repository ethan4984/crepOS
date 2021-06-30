#include <fs/fd.hpp>
#include <types.hpp>
#include <string.hpp>

namespace fs {

static size_t fd_cnt = 0;

fd::fd(lib::string path, int flags, int backing_fd) : status(0), backing_fd(backing_fd) {
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

int syscall_open(regs *regs_cur) {
    lib::string path((char*)regs_cur->rdi);
    int flags = regs_cur->rsi;
    
    fd &new_fd = (fd_list[fd_cnt] = fd(path, flags, fd_cnt));

    regs_cur->rax = fd_cnt++;

    if(new_fd.status == 0) {
        regs_cur->rax = -1;
        return -1;
    }

    return regs_cur->rax;
}

int syscall_read(regs *regs_cur) {
    fd &backing = fd_list[regs_cur->rdi];

    if(backing.backing_fd == -1) {
        set_errno(ebadf);
        regs_cur->rax = -1; 
        return -1;
    }

    regs_cur->rax = backing.read((void*)regs_cur->rsi, regs_cur->rdx);

    return regs_cur->rax;
}

int syscall_write(regs *regs_cur) {
    fd &backing = fd_list[regs_cur->rdi];

    if(backing.backing_fd == -1) {
        set_errno(ebadf);
        regs_cur->rax = -1; 
        return -1;
    }

    regs_cur->rax = backing.write((void*)regs_cur->rsi, regs_cur->rdx);

    return regs_cur->rax;
}

int syscall_seek(regs *regs_cur) {
    fd &backing = fd_list[regs_cur->rdi];

    if(backing.backing_fd == -1) {
        set_errno(ebadf);
        regs_cur->rax = -1; 
        return -1;
    }

    regs_cur->rax = backing.write((void*)regs_cur->rsi, regs_cur->rdx);

    return regs_cur->rax;
}

}
