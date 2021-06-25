#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <cstdint>
#include <cstddef>

using ssize_t = int64_t;
using off_t = int64_t;
using symbol = void*[]; 

using pid_t = ssize_t;
using tid_t = ssize_t;
using uid_t = ssize_t;
using gid_t = ssize_t;

using dev_t = uint64_t;
using ino_t =  uint64_t;
using mode_t = int32_t;
using nlink_t = int32_t;
using blksize_t = int64_t;
using blkcnt_t = int64_t;

using time_t = int64_t;
using clockid_t = int64_t;

namespace fd { 

constexpr size_t seek_cur = 1;
constexpr size_t seek_end = 2;
constexpr size_t seek_set = 3;

constexpr size_t o_accmode = 0x7;
constexpr size_t o_exec = 0x1;
constexpr size_t o_rdonly = 0x2;
constexpr size_t o_rdwr = 0x3;
constexpr size_t o_search = 0x4;
constexpr size_t o_wronly = 0x5;

constexpr size_t o_append = 0x8;
constexpr size_t o_creat = 0x10;
constexpr size_t o_directory = 0x20;
constexpr size_t o_excl = 0x40;
constexpr size_t o_noctty = 0x80;
constexpr size_t o_nofollow = 0x100;
constexpr size_t o_trunc = 0x200;
constexpr size_t o_nonblock = 0x400;
constexpr size_t o_dsync = 0x800;
constexpr size_t o_rsync = 0x1000;
constexpr size_t o_sync = 0x2000;
constexpr size_t o_cloexec = 0x4000;

};

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct stat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    timespec st_atim;
    timespec st_mtim;
    timespec st_ctim;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
};

#endif
