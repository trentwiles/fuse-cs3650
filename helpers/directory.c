// directory.c

#include "directory.h"
#include "inode.h"
#include "blocks.h"
#include "slist.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_DIR_ENTRIES (BLOCK_SIZE / sizeof(dirent_t))
#define S_IFDIR 0040000

static inode_t *root_inode = NULL;

void directory_init() {
    int root_inum = alloc_inode();
    if (root_inum < 0) {
        fprintf(stderr, "Failed to allocate root inode.\n");
        exit(EXIT_FAILURE);
    }
    root_inode = get_inode(root_inum);
    root_inode->mode = S_IFDIR | 0755;
    root_inode->size = 0;
    root_inode->block = allocate_block(1);
    if (root_inode->block < 0) {
        fprintf(stderr, "Failed to allocate block for root directory.\n");
        exit(EXIT_FAILURE);
    }
}

int directory_lookup(inode_t *di, const char *name) {
    if (di == NULL) di = root_inode;
    if (!(di->mode & S_IFDIR)) return -1;

    char *data = (char *)get_block(di->block);
    if (!data) return -1;

    for (int i = 0; i < di->size / sizeof(dirent_t); i++) {
        dirent_t *entry = (dirent_t *)(data + i * sizeof(dirent_t));
        if (strcmp(entry->name, name) == 0) {
            return entry->inum;
        }
    }
    return -1;
}

int directory_put(inode_t *di, const char *name, int inum) {
    if (di == NULL) di = root_inode;
    if (!(di->mode & S_IFDIR)) return -1;

    if (directory_lookup(di, name) != -1) return -1; // Entry already exists

    dirent_t new_entry;
    strncpy(new_entry.name, name, DIR_NAME_LENGTH);
    new_entry.inum = inum;
    memset(new_entry._reserved, 0, sizeof(new_entry._reserved));

    int entries = di->size / sizeof(dirent_t);
    if (entries >= MAX_DIR_ENTRIES) return -1; // Directory full

    char *data = (char *)get_block(di->block);
    if (!data) return -1;

    memcpy(data + entries * sizeof(dirent_t), &new_entry, sizeof(dirent_t));
    di->size += sizeof(dirent_t);
    return 0;
}

int directory_delete(inode_t *di, const char *name) {
    if (di == NULL) di = root_inode;
    if (!(di->mode & S_IFDIR)) return -1;

    char *data = (char *)get_block(di->block);
    if (!data) return -1;

    int entries = di->size / sizeof(dirent_t);
    for (int i = 0; i < entries; i++) {
        dirent_t *entry = (dirent_t *)(data + i * sizeof(dirent_t));
        if (strcmp(entry->name, name) == 0) {
            // Move the last entry to the current spot
            dirent_t *last_entry = (dirent_t *)(data + (entries - 1) * sizeof(dirent_t));
            memcpy(entry, last_entry, sizeof(dirent_t));
            di->size -= sizeof(dirent_t);
            return 0;
        }
    }
    return -1; // Entry not found
}

slist_t *directory_list(const char *path) {
    inode_t *di = get_inode(directory_lookup(NULL, path));
    if (!di || !(di->mode & S_IFDIR)) return NULL;

    slist_t *list = NULL;
    char *data = (char *)get_block(di->block);
    if (!data) return NULL;

    int entries = di->size / sizeof(dirent_t);
    for (int i = 0; i < entries; i++) {
        dirent_t *entry = (dirent_t *)(data + i * sizeof(dirent_t));
        list = slist_cons(entry->name, list);
    }
    return list;
}

void print_directory(inode_t *dd) {
    if (!dd || !(dd->mode & S_IFDIR)) {
        printf("Not a directory.\n");
        return;
    }

    char *data = (char *)get_block(dd->block);
    if (!data) {
        printf("Failed to read directory data.\n");
        return;
    }

    int entries = dd->size / sizeof(dirent_t);
    printf("Directory contents:\n");
    for (int i = 0; i < entries; i++) {
        dirent_t *entry = (dirent_t *)(data + i * sizeof(dirent_t));
        printf("Name: %s, Inode: %d\n", entry->name, entry->inum);
    }
}