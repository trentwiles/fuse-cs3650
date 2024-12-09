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

// directory insertion
int directory_put(inode_t* directory_inode, const char* name, int inum) {
    // variable for how many entries currently exist in this directory
    int entry_count = directory_inode->size / sizeof(dirent);

    // get directory entries from inode
    dirent* entries = blocks_get_block(directory_inode->ptrs[0]);
    if (entries == NULL) {
        // basic error handling
        return -1;
    }

    dirent new_entry;
    strncpy(new_entry.name, name, DIR_NAME_LENGTH);
    // can't forget null termination
    new_entry.name[DIR_NAME_LENGTH - 1] = '\0';
    new_entry.inum = inum;
    new_entry.used = 1;

    // search for an unused slot in our entrylist
    // starting at 1 ignores special entries ("." & "..")
    int allocated = 0;
    for (int x = 1; x < entry_count; x++) {
        if (!entries[x].used) {
            entries[x] = new_entry;
            allocated = 1;
            // exit when entry is placed
            break;
        }
    }

    // otherwise, new entry at the end
    if (!allocated) {
        entries[entry_count] = new_entry;
        directory_inode->size += sizeof(dirent);
    }

    printf("DEBUG for directory_put func: inserted \"%s\" (inum=%d) into block %d\n", name, inum, directory_inode->ptrs[0]);

    return 0;
}

// deletion of a directory function
int directory_delete(inode_t* directory_inode, const char* name) {
    // grab entries block
    dirent* entries = blocks_get_block(directory_inode->ptrs[0]);
    if (entries == NULL) {
        // can't find it? return an error
        return -EIO; 
    }

    // how many entries are in the directory
    int entry_count = directory_inode->size / sizeof(dirent);

    // find entry that matches name
    for (int i = 0; i < entry_count; i++) {
        if (entries[i].used && strcmp(entries[i].name, name) == 0) {
            // mark unused, call helper method to reduce reference count
            entries[i].used = 0;
            decrease_refs(entries[i].inum);
            return 0;
        }
    }

    // if all else fails return no such file/directory error
    return -ENOENT;
}

slist_t* directory_list(const char* path) {
    // inode # of the directory
    int directory_inum = tree_lookup(path);
    if (directory_inum == -1) {
        // should the path not exist, return NULL
        // to indicate eror
        return NULL;
    }

    // Retrieve the inode for the given directory
    inode_t* directory_inode = get_inode(directory_inum);
    if (directory_inode == NULL) {
        // If we failed to get the inode, return NULL or handle the error appropriately
        return NULL;
    }

    // Calculate how many directory entries there are
    int entry_count = directory_inode->size / sizeof(dirent);

    // Retrieve the directory entries
    dirent* entries = blocks_get_block(directory_inode->ptrs[0]);
    if (entries == NULL) {
        // If we can't retrieve directory entries, return NULL
        return NULL;
    }

    // Initialize a list to store directory names
    slist_t* dirnames = NULL;

    // Iterate through all directory entries and add the names of 'used' entries to the list
    for (int i = 0; i < entry_count; i++) {
        if (entries[i].used) {
            // Prepend the current entry name to the list
            dirnames = slist_cons(entries[i].name, dirnames);
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

