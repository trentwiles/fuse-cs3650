// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

// including blocks.h isn't technically needed, but we can remove later
#include "blocks.h"

#define nptrs 2

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[nptrs]; // direct pointers
    int iptr; // single indirect pointer
} inode_t;

void print_inode(inode_t* node);
inode_t* get_inode(int inum);
int alloc_inode();
void free_inode(int inum);
int grow_inode(inode_t* node, int size);
int shrink_inode(inode_t* node, int size);
int inode_get_pnum(inode_t* node, int fpn);
void decrease_refs(int inum);

#endif
