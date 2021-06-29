#include <fs/vfs.hpp>
#include <fs/devfs.hpp>
 
namespace vfs {
 
node::node(lib::string absolute_path, lib::string relative_path, lib::string name, fs *filesystem, default_ioctl *ioctl_device) :
    absolute_path(absolute_path),
    relative_path(relative_path),
    name(name),
    filesystem(filesystem),
    ioctl_device(ioctl_device) {
    if(filesystem == NULL) {
        filesystem = new fs;
        *filesystem = fs(fs::nofs_signature);
    }
 
    parent = this;
}
 
node::node(lib::string absolute_path, default_ioctl *ioctl_device) : absolute_path(absolute_path), ioctl_device(ioctl_device) {
    if(absolute_path[0] == '/') 
        absolute_path++;
 
    lib::vector<lib::string> sub_paths = [&]() {
        size_t start = 0;
        size_t end = absolute_path.find_first('/');
 
        lib::vector<lib::string> ret;
 
        while(end != lib::string::npos) {
            lib::string token = absolute_path.substr(start, end - start);
            if(!token.empty())
                ret.push(token);
            start = end + 1;
            end = absolute_path.find_first('/', start);
        }

        lib::string token = absolute_path.substr(start, end);
        if(!token.empty())
            ret.push(absolute_path.substr(start, end));
 
        return ret;
    } ();

    node *node_cur = &root_node; 
    node *parent_cur;

    for(size_t i = 0; i < sub_paths.size(); i++) {
        parent_cur = node_cur;
        node_cur = node_cur->search_relative(sub_paths[i]);
        if(node_cur == NULL) {
            do {
                node *child = create_node(parent_cur, sub_paths[i]);
                parent_cur->child = child;
                parent_cur = child;
            } while(++i < sub_paths.size());
            return;
        }
    }
}
 
node *node::search_relative(lib::string name) {
    node *cur = parent->next;
    
    while(cur != NULL) {
        if(cur->name == name)
            return cur;
        cur = cur->next;
    }
 
    return NULL;
}
 
node *node::search_absolute(lib::string path) {
    if(path == "/")
        return &root_node;
    
    if(path[0] == '/')
        path++;
 
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

    node *cur = &root_node;
 
    for(size_t i = 0; i < sub_paths.size(); i++) {
        cur = cur->search_relative(sub_paths[i]);
        if(cur == NULL) {
            return NULL;
        }
    }
 
    return cur;
}

node *create_node(node *parent, lib::string name) {
    if(parent == NULL)
        parent = &root_node;
 
    lib::string absolute_path;
 
    if(parent != &root_node) {
        absolute_path = parent->absolute_path;
        absolute_path += "/";
        absolute_path += name;
    }
    else
        absolute_path = parent->absolute_path + name;

    lib::string relative_path("");
    if(parent->filesystem != NULL && parent->filesystem->flags & fs::is_mounted) {
        relative_path = lib::string(absolute_path.data() + parent->filesystem->mount_gate.length());
    }
 
    node *new_node = reinterpret_cast<node*>(kmm::calloc(sizeof(node)));
 
    new_node->absolute_path = absolute_path;
    new_node->relative_path = relative_path;
    new_node->name = name;
    new_node->filesystem = parent->filesystem;
    new_node->stat_cur = (stat*)kmm::calloc(sizeof(stat));
 
    new_node->parent = parent;
    node *cur = parent;
 
    while(cur) { 
        if(cur->next == NULL) {
            cur->next = new_node;
            new_node->last = cur;
            return new_node;
        }
        cur = cur->next;
    } 
 
    return NULL;
}
 
void node::ioctl(regs *regs_cur) {
    if(ioctl_device == NULL)
        return;
    ioctl_device->call(regs_cur);
}
 
void node::remove(lib::string path) {
    node *cur = search_absolute(path);
    if(cur == NULL)
        return;
 
    remove_cluster(cur);
}
 
void node::remove_cluster(node *cur) {
    while(cur) {
        if(cur->child) {
            remove_cluster(cur->child);
        }
        cur->last->next = 0;
        delete cur;
        cur = cur->next;
    }
}

int mount(lib::string source, lib::string target) {
    node *source_vfs_node = root_node.search_absolute(source);
    if(source_vfs_node == NULL) 
        return -1;
    
    dev::node source_devfs_node(source_vfs_node);
    if(source_devfs_node.vfs_node == NULL) 
        return -1;

    if(source_devfs_node.filesystem == NULL)
        return -1;

    node *target_vfs_node = root_node.search_absolute(target);
    if(target_vfs_node == NULL) 
        return -1;

    target_vfs_node->filesystem = source_devfs_node.filesystem;
    target_vfs_node->filesystem->mount_gate = target;
    target_vfs_node->filesystem->flags |= vfs::fs::is_mounted;
    target_vfs_node->filesystem->refresh(target_vfs_node);

    return 0;
}
 
}
