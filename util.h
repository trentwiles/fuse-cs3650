// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include "blocks.h"
#include <time.h>

#define nptrs 2
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[nptrs]; // direct pointers
    int iptr; // single indirect pointer
    time_t atim; // time last accessed
    time_t ctim; // time created
    time_t mtim; // time last modified
} inode;

void print_inode(inode* node);
inode* get_inode(int inum);
int alloc_inode();
void free_inode(int inum);
int grow_inode(inode* node, int size);
int shrink_inode(inode* node, int size);
int inode_get_pnum(inode* node, int fpn);
void decrease_refs(int inum);

#endif
