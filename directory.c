#include "slist.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"
#include "bitmap.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>

void directory_init() {
    // we already have our page for the inodes allocated
    // the root inode will be node 0
    inode_t* rootnode = get_inode(alloc_inode());
    rootnode->mode = 040755;
}

int directory_lookup(inode_t* directory_inode, const char* name) {
    // name empty = root directory
    if (strcmp(name, "") == 0) {
        return 0;
    }

    // get first directory block, that contains the directory entries
    dirent* entries = blocks_get_block(directory_inode->ptrs[0]);
    // included since system might NULL on error
    if (entries == NULL) { return -1; }

    // based on fixed number of entries (64)
    const int ENTRY_COUNT = 64;
    for (int i = 0; i < ENTRY_COUNT; ++i) {
        dirent current_entry = entries[i];

        // ensure directory name is in use (we added the .used to ferd's code)
        if (current_entry.used && strcmp(name, current_entry.name) == 0) {
            return current_entry.inum;  // Found the matching entry
        }
    }

    // if all checks fail, there was no directory found
    return -1;
}

int tree_lookup(const char* path) {
    // Start from the root node
    int current_node = 0;
    
    // path spliting
    // "test/new" -> "test", "new"
    slist_t* path_components = slist_explode(path, '/');
    slist_t* current_component = path_components;
    
    // traverse through directory
    while (current_component != NULL) {
        current_node = directory_lookup(get_inode(current_node), current_component->data);
        if (current_node == -1) {
            // not found? free the list and return -1
            slist_free(path_components);
            return -1;
        }
        
        current_component = current_component->next;
    }

    // post traversial cleanup
    slist_free(path_components);
    printf("tree lookup: %s is at node %d\n", path, current_node);
    return current_node;
}

    
int directory_put(inode_t* dd, const char* name, int inum) {
    int numentries = dd->size / sizeof(dirent);
    dirent* entries = blocks_get_block(dd->ptrs[0]);
    int alloced = 0; // we want to keep track of whether we've alloced or not

    // building the new directory entry;
    dirent new;
    strncpy(new.name, name, DIR_NAME_LENGTH); 
    new.inum = inum;
    new.used = 1;

    for (int ii = 1; ii < dd->size / sizeof(dirent); ++ii) {
        if (entries[ii].used == 0) {
            entries[ii] = new;
            alloced = 1;
        }
    }
    
    if (!alloced) {
        entries[numentries] = new;
        dd->size = dd->size + sizeof(dirent);
    }

    printf("running dir_put, putting %s, inum %d, on page %d\n", name, inum, dd->ptrs[0]);
    return 0;
}

// this sets the matching directory to unused and takes a ref off its inode
int directory_delete(inode_t* dd, const char* name) {
    printf("running dir delete on filename %s\n", name);
    dirent* entries = blocks_get_block(dd->ptrs[0]);
    printf("got direntries at block %d\n", dd->ptrs[0]);
    for (int ii = 0; ii < dd->size / sizeof(dirent); ++ii) {
        if (strcmp(entries[ii].name, name) == 0) {
            printf("found a deletion match at entry %d\n", ii);
            entries[ii].used = 0;
            decrease_refs(entries[ii].inum);
            return 0;
        }
    }
    printf("no file found! cannot delete");
    return -ENOENT;
}

// list of directories where? at the path? wait... this is for ls
slist_t* directory_list(const char* path) {
    int working_dir = tree_lookup(path);
    inode_t* w_inode = get_inode(working_dir);
    int numdirs = w_inode->size / sizeof(dirent);
    dirent* dirs = blocks_get_block(w_inode->ptrs[0]);
    slist_t* dirnames = NULL; 
    for (int ii = 0; ii < numdirs; ++ii) {
        if (dirs[ii].used) {
            dirnames = slist_cons(dirs[ii].name, dirnames);
        }
    }
    return dirnames;
}


void print_directory(inode_t* dd) {
    int numdirs = dd->size / sizeof(dirent);
    dirent* dirs = blocks_get_block(dd->ptrs[0]);
    for (int ii = 0; ii < numdirs; ++ii) {
        printf("%s\n", dirs[ii].name);
    }
}

