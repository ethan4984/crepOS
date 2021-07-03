#ifndef FD_HPP_
#define FD_HPP_

#include <fs/vfs.hpp>

namespace fs {

struct fd {
    fd(lib::string path, int flags, int backing_fd);
    fd(int backing_fd);
    fd() : status(0), backing_fd(-1) { }

    int read(void *buf, size_t cnt);
    int write(void *buf, size_t cnt);
    int seek(off_t off, int whence);

    int status;
    int backing_fd;
    
    size_t *_loc;
    size_t *_flags;

    vfs::node *vfs_node;
};

inline lib::map<ssize_t, fd> fd_list;

}

#endif
