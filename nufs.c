// based on cs3650 starter code

#include <assert.h>
#include <bsd/string.h>
// #include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "inode.h"
#include "storage.h"

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
  // real implementation based on ferd's code
  int rv = storage_access(path);

  // debugging statement from ferd's
  // probably not needed, delete at some point
  printf("access(%s, %04o) -> %d\n", path, mask, rv);
  return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = 0;

  // root directory metadata
  if (strcmp(path, "/") == 0) {
    st->st_mode = 040755; // directory
    st->st_size = 0;
    st->st_uid = getuid();
  } else {
    rv = storage_stat(path, st);
    st->st_uid = getuid();
  }

  printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode,
         st->st_size);

  if (rv == -1) {
    return -ENOENT;
  }
  return 0;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  struct stat st;
  int rv;

  rv = nufs_getattr(path, &st);
  assert(rv == 0);

  slist_t *dirnames = storage_list(path);
  filler(buf, ".", &st, 0);
  if (dirnames == NULL) {
    return 0;
  }

  slist_t *currname = dirnames;
  while (currname != NULL) {
    char currpath[strlen(path) + 50];
    strncpy(currpath, path, strlen(path));
    if (path[strlen(path) - 1] == '/') {
      currpath[strlen(path)] = 0;
    } else {
      currpath[strlen(path)] = '/';
      currpath[strlen(path) + 1] = 0;
    }
    strncat(currpath, currname->data, 48);
    nufs_getattr(currpath, &st);
    filler(buf, currname->data, &st, 0);
    currname = currname->next;
  }

  printf("readdir(%s) -> %d\n", path, rv);
  slist_free(dirnames);
  return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
// ^^^ we decided against that
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  // simply makes a call to our other method
  int rv = storage_mknod(path, mode);
  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = nufs_mknod(path, mode | 040000, 0);
  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_unlink(const char *path) {
  int rv = storage_unlink(path);
  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_link(const char *from, const char *to) {
  int rv = -1;
  printf("link(%s => %s) -> %d\n", from, to, rv);
  rv = storage_link(to, from);
  return rv;
}

int nufs_rmdir(const char *path) {
  int rv = -1;

  // is directory empty?
  slist_t *contents = storage_list(path);
  if (contents != NULL && contents->next != NULL) {
    printf("rmdir(%s) -> %d (directory not empty)\n", path, -ENOTEMPTY);
    slist_free(contents);
    return -ENOTEMPTY;
  }

  // remove via storage layer if needed
  rv = storage_rmdir(path);
  slist_free(contents);

  if (rv == -1) {
    printf("rmdir(%s) -> %d (directory does not exist / error occurred)\n",
           path, -ENOENT);
    return -ENOENT;
  }

  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {
  int rv = storage_rename(from, to);
  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

// dummy implementation
int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// called to change the size of a file
int nufs_truncate(const char *path, off_t size) {
  int rv = storage_truncate(path, size);
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = 0;
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = -1;
  rv = storage_read(path, buf, size, offset);
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  int rv = -1;
  rv = storage_write(path, buf, size, offset);
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

/**
 * Based on office hours, we removed the extended operations, as they are not
 * needed. Now they just return dummy data.
 */

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) { return 0; }

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  return 0;
}

void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  // printf("TODO: mount %s as data file\n", argv[--argc]);
  storage_init(argv[--argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
