// Inode manipulation routines.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

// including blocks.h isn't technically needed, but we can remove later
#include "blocks.h"

#define nptrs 2

typedef struct inode {
  int refs;        // reference count
  int mode;        // permission & type
  int size;        // bytes
  int ptrs[nptrs]; // direct pointers
  int iptr;        // single indirect pointer
} inode_t; // instead of having a single block pointer, we have an array of them

void print_inode(inode_t *node);
inode_t *get_inode(int inum);
int alloc_inode();
void free_inode(int inum);
int grow_inode(inode_t *node, int size);
int shrink_inode(inode_t *node, int size);
int inode_get_bnum(inode_t *node, int file_bnum);
void decrease_refs(int inum);

#endif
