#include <fs/ext2/ext2.hpp> 
 
namespace ext2 {

ssize_t dir::search_relative(lib::string path) {
    uint8_t *buffer = new uint8_t[parent_inode->raw.size32l];
    parent_inode->read(0, parent_inode->raw.size32l, buffer);

    for(uint32_t i = 0; i < parent_inode->raw.size32l;) { 
        raw_dir *dir_cur = reinterpret_cast<raw_dir*>(buffer + i);

        lib::string name(reinterpret_cast<char*>(dir_cur->name), dir_cur->name_length);

        if(path == name) {
            if(dir_cur->inode == 0) {
                delete buffer;
                return -1;
            }

            raw = dir_cur;

            return 0;
        }

        uint32_t expected_size = align_up(sizeof(raw_dir) + dir_cur->name_length, 4);
        if(dir_cur->entry_size != expected_size)
            break;

        i += dir_cur->entry_size;
    }

    delete buffer;
    return -1;
}

dir::dir(inode *parent_inode, lib::string path, bool find) : raw(NULL), parent_inode(parent_inode), path(path), exists(false) {
    lib::vector<lib::string> sub_paths = [&]() {
        size_t start = 0;
        size_t end = path.find_first('/');
 
        lib::vector<lib::string> ret;
 
        while(end != lib::string::npos) {
            lib::string token = path.substr(start, end - start);
            if(!token.empty())
                ret.push(token);
            start = end + 1;
            end = path.find_first('/', start);
        }

        lib::string token = path.substr(start, end);
        if(!token.empty())
            ret.push(path.substr(start, end));
 
        return ret;
    } ();

    if(find == true) {
        inode *save = parent_inode;
        parent_inode = new inode;
        *parent_inode = *save;

        for(size_t i = 0; i < sub_paths.size(); i++) {
            if(search_relative(sub_paths[i]) == -1) {
                exists = false;
                return;
            }
            *parent_inode = inode(parent_inode->parent, raw->inode);
        }

        delete parent_inode;
        parent_inode = save;
    } else {
        if(delete_relative(sub_paths.last()) == -1) {
            exists = false;
            return;
        }
    }

    exists = true;
}

dir::dir(inode *parent_inode, uint32_t new_inode, uint8_t type, char *name) : parent_inode(parent_inode), exists(false) {
    uint8_t *buffer = new uint8_t[parent_inode->raw.size32l];
    parent_inode->read(0, parent_inode->raw.size32l, buffer);

    bool found = false;

    for(uint32_t i = 0; i < parent_inode->raw.size32l;) { 
        raw_dir *dir_cur = reinterpret_cast<raw_dir*>(buffer + i);

        if(found) {
            dir_cur->inode = new_inode; 
            dir_cur->type = type;
            dir_cur->name_length = strlen(name);
            dir_cur->entry_size = parent_inode->parent->block_size - i;

            memcpy8((uint8_t*)dir_cur->name, (uint8_t*)name, dir_cur->name_length);

            i += dir_cur->entry_size;
            dir_cur = (raw_dir*)(buffer + i);
            memset8((uint8_t*)dir_cur, 0, sizeof(raw_dir));

            parent_inode->write(0, parent_inode->raw.size32l, buffer);
            
            exists = true;
        }

        uint32_t expected_size = align_up(sizeof(raw_dir) + dir_cur->name_length, 4);
        if(dir_cur->entry_size != expected_size) {
            dir_cur->entry_size = expected_size;
            i += expected_size; 
            found = 1;
            continue; 
        }

        i += dir_cur->entry_size;
    }
}

ssize_t dir::delete_relative(lib::string path) {
    uint8_t *buffer = new uint8_t[parent_inode->raw.size32l];
    parent_inode->read(0, parent_inode->raw.size32l, buffer);

    for(uint32_t i = 0; i < parent_inode->raw.size32l;) { 
        raw_dir *dir_cur = reinterpret_cast<raw_dir*>(buffer + i);

        lib::string name(reinterpret_cast<char*>(dir_cur->name), dir_cur->name_length);

        if(name == path) {
            memset8((uint8_t*)dir_cur->name, 0, dir_cur->name_length); 
            inode dir_parent_inode(parent_inode->parent, dir_cur->inode);
            dir_parent_inode.remove();
            dir_cur->inode = 0;
            parent_inode->write(0, parent_inode->raw.size32l, buffer);

            return 0;
        }

        uint32_t expected_size = align_up(sizeof(raw_dir) + dir_cur->name_length, 4);
        if(dir_cur->entry_size != expected_size)
            break;

        i += dir_cur->entry_size;
    }
    return -1;
}

}
