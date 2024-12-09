// storage.c

#include "storage.h"
#include "directory.h"
#include "inode.h"
#include "bitmap.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define PATH_MAX 4096
static char storage_path[PATH_MAX];

void storage_init(const char *path) {
    strncpy(storage_path, path, PATH_MAX);
    directory_init();
}

int storage_stat(const char *path, struct stat *st) {
    inode_t *inode = get_inode(directory_lookup(NULL, path));
    if (!inode) return -1;
    st->st_mode = inode->mode;
    st->st_size = inode->size;
    // Populate other stat fields as needed
    return 0;
}

int storage_read(const char *path, char *buf, size_t size, off_t offset) {
    inode_t *inode = get_inode(directory_lookup(NULL, path));
    if (!inode) return -1;
    if (offset + size > inode->size) return -1;
    // Read from inode->block + offset
    // Assuming direct block
    memcpy(buf, (char *)inode->block + offset, size);
    return size;
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    inode_t *inode = get_inode(directory_lookup(NULL, path));
    if (!inode) return -1;
    if (offset + size > 4096) return -1; // Assuming 4K block
    memcpy((char *)inode->block + offset, buf, size);
    inode->size = (offset + size) > inode->size ? (offset + size) : inode->size;
    return size;
}

int storage_truncate(const char *path, off_t size) {
    inode_t *inode = get_inode(directory_lookup(NULL, path));
    if (!inode) return -1;
    if (size > 4096) return -1; // Assuming 4K block
    inode->size = size;
    return 0;
}

int storage_mknod(const char *path, int mode) {
    int inum = alloc_inode();
    if (inum < 0) return -1;
    inode_t *inode = get_inode(inum);
    inode->mode = mode;
    inode->size = 0;
    inode->block = 0;
    return directory_put(NULL, path, inum);
}

int storage_unlink(const char *path) {
    inode_t *inode = get_inode(directory_lookup(NULL, path));
    if (!inode) return -1;
    free_inode();
    return directory_delete(NULL, path);
}

int storage_link(const char *from, const char *to) {
    inode_t *inode = get_inode(directory_lookup(NULL, from));
    if (!inode) return -1;
    inode->refs++;
    return directory_put(NULL, to, inode->refs);
}

int storage_rename(const char *from, const char *to) {
    int inum = directory_lookup(NULL, from);
    if (inum < 0) return -1;
    directory_delete(NULL, from);
    return directory_put(NULL, to, inum);
}

int storage_set_time(const char *path, const struct timespec ts[2]) {
    // Implement time setting on inode
    return 0;
}

int storage_chmod(const char *path, mode_t mode) {
    inode_t *inode = get_inode(directory_lookup(NULL, path));
    if (!inode) return -1;
    inode->mode = mode;
    return 0;
}

int storage_access(const char *path, int mask) {
    inode_t *inode = get_inode(directory_lookup(NULL, path));
    if (!inode) return -1;
    return (inode->mode & mask) ? 0 : -1;
}

slist_t *storage_list(const char *path) {
    return directory_list(path);
}