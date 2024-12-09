// Directory manipulation functions.
//
// Feel free to use as inspiration. Provided as-is.

// Based on cs3650 starter code
#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

typedef struct dirent {
  char name[DIR_NAME_LENGTH];
  int inum;
  char used; // added to ferd's implementation to determine if dir is in use
  char _reserved[12];
} dirent;

void directory_init();
int directory_lookup(inode *di, const char *name);
// useful function for discovering a path's location
int tree_lookup(const char* path);

// we also changed inode_t to inode to fit our implementation
// yet another design choice
int directory_put(inode *di, const char *name, int inum);
int directory_delete(inode *di, const char *name);
slist_t *directory_list(const char *path);
void print_directory(inode *dd);

#endif