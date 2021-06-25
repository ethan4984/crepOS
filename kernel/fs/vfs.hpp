#ifndef VFS_HPP_
#define VFS_HPP_

#include <string.hpp>
#include <map.hpp>

namespace vfs {

struct default_ioctl { 
    virtual int call([[maybe_unused]] regs *regs_cur) { 
        print("Warning: this fd is not assoicated with any device\n");
        return -1; 
    }
};

struct fs;

class node {
public:
    node(lib::string absolute_path, lib::string relative_path, lib::string name, fs *fs_cur, default_ioctl *ioctl_device = NULL);
    node(lib::string absolute_path, default_ioctl *ioctl_device = NULL);
    node() = default;

    lib::string absolute_path;
    lib::string relative_path;
    lib::string name;

    fs *filesystem;
    stat *stat_cur;

    void ioctl(regs *regs_cur);
    node *search_absolute(lib::string path);
    node *search_relative(lib::string name);
    void remove(lib::string path);

    friend node *create_node(node *parent, lib::string name);
private:
    node *next; // next file in the current directory
    node *last; // last file in the current directory

    node *child; // child files
    node *parent; // parent directory

    default_ioctl *ioctl_device;

    void remove_cluster(node *cur);
};

inline node root_node("/", "/", "/", NULL);

struct fs {
    explicit fs(size_t flags) : flags(flags) { }
    explicit fs() = default;

    virtual int read(node *vfs_node, off_t off, off_t cnt, void *buf) {
        print("Warning: unimplemented filesystem call on node {} read<{}, {}, {}>\n",   vfs_node->absolute_path,
                                                                                        off,
                                                                                        cnt,
                                                                                        reinterpret_cast<size_t>(buf));
        return -1;
    }

    virtual int write(node *vfs_node, off_t off, off_t cnt, const void *buf) {
        print("Warning: unimplemented filesystem call on node {} write<{}, {}, {}>\n",  vfs_node->absolute_path,
                                                                                        off,
                                                                                        cnt,
                                                                                        reinterpret_cast<size_t>(buf));
        return -1;
    }

    virtual int mkdir(node *vfs_node) {
        print("Warning: unimplemented filesystem call on node {} mkdir<>\n", vfs_node->absolute_path);
        return -1;
    }

    virtual int refresh(node *vfs_node) {
        print("Warning: unimplemented filesystem call on node {} refresh<>\n", vfs_node->absolute_path);
        return -1;
    }

    virtual int open(node *vfs_node, uint16_t status) {
        print("Warning: unimplemented filesystem call on node {} open<{}>\n", vfs_node->absolute_path, status);
        return -1;
    }

    virtual int unlink(node *vfs_node) {
        print("Warning: unimplemented filesystem call on node {} unlink<>\n", vfs_node->absolute_path);
        return -1;
    }

    lib::string mount_gate;

    static constexpr size_t nofs_signature = (0 << 0);
    static constexpr size_t ext2_signature = (0 << 1);
    static constexpr size_t fat32_signature = (0 << 2);
    static constexpr size_t ramfs_signature = (0 << 3);
    static constexpr size_t is_mounted = (0 << 4);

    size_t flags;
};

void test();

}

#endif
