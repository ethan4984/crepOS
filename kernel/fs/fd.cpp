#include <fs/fd.hpp>
#include <types.hpp>
#include <string.hpp>
#include <sched/smp.hpp>
#include <sched/scheduler.hpp>
#include <memutils.hpp>
#include <utility>

#define SYSCALL_FD_TRANSLATE(FD_INDEX) \
    auto status = translate((FD_INDEX)); \
    if(status.first == -1) { \
        set_errno(ebadf); \
        regs_cur->rax = -1; \
        return -1; \
    } \

namespace fs {

static fd &alloc_fd(lib::string path, int flags) {
    smp::cpu &core = smp::core_local();
    sched::task &current_task = sched::task_list[core.pid];

    const auto index = [](sched::task &task) {
        const auto find_index = [](sched::task &task, const auto &func) -> size_t {
            for(size_t i = 0; i < task.fd_list.bitmap_size; i++) {
                if(!bm_test(task.fd_list.bitmap, i)) {
                    bm_set(task.fd_list.bitmap, i);
                    return i;
                }
            }

            task.fd_list.bitmap_size += 0x1000;
            task.fd_list.bitmap = (uint8_t*)kmm::recalloc(task.fd_list.bitmap, task.fd_list.bitmap_size);

            return func(task, func);
        }; 

        return find_index(task, find_index);
    } (current_task);

    return (fd_list[index] = fd(path, flags, index));
}

std::pair<ssize_t, fd&&> translate(int index) {
    std::pair<ssize_t, fd&&> ret = { .first = -1, .second = fd() };

    fd &backing = fd_list[index];

    if(backing.backing_fd == -1 || backing.status == 0)
        return ret;

    ret.first = 0;
    ret.second = backing;

    return ret;
}

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

extern "C" int syscall_open(regs *regs_cur) {
    lib::string path((char*)regs_cur->rdi);
    int flags = regs_cur->rsi;

    fd &new_fd = alloc_fd(path, flags);

    regs_cur->rax = new_fd.backing_fd;

    if(new_fd.status == 0) {
        regs_cur->rax = -1;
        return -1;
    }

    return regs_cur->rax;
}

extern "C" int syscall_close(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    fd_list.remove(regs_cur->rdi);

    return (regs_cur->rax = 0);
}

extern "C" int syscall_read(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    return (regs_cur->rax = status.second.read((void*)regs_cur->rsi, regs_cur->rdx));
}

extern "C" int syscall_write(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    return (regs_cur->rax = status.second.write((void*)regs_cur->rsi, regs_cur->rdx));
}

extern "C" int syscall_seek(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    return (regs_cur->rax = status.second.write((void*)regs_cur->rsi, regs_cur->rdx));
}

extern "C" int syscall_dup(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    return (regs_cur->rax = alloc_fd(status.second.vfs_node->absolute_path, *status.second._flags).backing_fd); 
}

extern "C" int syscall_dup2(regs *regs_cur) {
    if(regs_cur->rdi == regs_cur->rsi)
        return regs_cur->rsi;

    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    auto new_fd_status = translate(regs_cur->rsi);
    if(new_fd_status.first != -1)
        fd_list.remove(regs_cur->rsi);

    fd_list[regs_cur->rsi] = status.second;

    return regs_cur->rsi;
}

}
