#ifndef FD_HPP_
#define FD_HPP_

#include <fs/vfs.hpp>

namespace fs {

class fd {
public:
    fd(lib::string path, int flags);
    fd() = default;

    int read(void *buf, size_t cnt);
    int write(void *buf, size_t cnt);
    int seek(off_t off, int whence);

    int status;
protected:
    ssize_t _backing_fd;
    size_t *_loc;
    size_t *_flags;

    vfs::node *vfs_node;
};

inline lib::map<ssize_t, fd> fd_list;

}

#endif
