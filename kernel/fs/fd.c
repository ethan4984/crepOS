#include <fs/fd.h>
#include <types.h>

static_vec(fd_t, fd_list);

int open(char *path, int flags) {
    if(vfs_open(path, flags) == -1) 
        return -1;

    vfs_node_t *node = vfs_absolute_path(path);

    if(node == NULL)
        return -1;

    fd_t fd = { .vfs_node = node,
                .flags = kmalloc(sizeof(int)),
                .loc = kmalloc(sizeof(size_t))
              };

    *fd.flags = flags;
    *fd.loc = 0;

    return vec_push(fd_t, fd_list, fd);
}

int read(int fd, void *buf, size_t cnt) {
    fd_t *fd_entry = vec_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;

    int ret = vfs_read(fd_entry->vfs_node, *fd_entry->loc, cnt, buf);
    if(ret == -1) 
        return -1;

    *fd_entry->loc += cnt;

    return ret;
}

int write(int fd, void *buf, size_t cnt) {
    fd_t *fd_entry = vec_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;

    int ret = vfs_write(fd_entry->vfs_node, *fd_entry->loc, cnt, buf);
    if(ret == -1) 
        return -1;

    *fd_entry->loc += cnt;

    return ret;
}

int lseek(int fd, off_t off, int whence) {
    fd_t *fd_entry = vec_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;

    switch(whence) {
        case SEEK_SET:
            return (*fd_entry->loc = off); 
        case SEEK_CUR:
            return (*fd_entry->loc += off);
        case SEEK_END:
            return (*fd_entry->loc = fd_entry->vfs_node->stat.st_size + off);
    }

    return -1;
}

int close(int fd) {
    fd_t *fd_entry = vec_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;
    
    return vec_addr_remove(fd_t, fd_list, fd_entry);
}

int dup(int fd) {
    fd_t *fd_entry = vec_search(fd_t, fd_list, (size_t)fd);
    if(fd_entry == NULL)
        return -1;

    fd_t new_fd = *fd_entry;

    return vec_push(fd_t, fd_list, new_fd);
}

int dup2(int old_fd, int new_fd) {
    fd_t *old_fd_entry = vec_search(fd_t, fd_list, (size_t)old_fd);
    if(old_fd_entry == NULL)
        return -1;

    fd_t *new_fd_entry = vec_search(fd_t, fd_list, (size_t)new_fd);
    if(new_fd_entry != NULL) {
        close(new_fd);
    }

    fd_t fd = *old_fd_entry;

    return vec_push(fd_t, fd_list, fd);
}
