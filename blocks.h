// based on cs3650 starter code

#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdio.h>

void blocks_init(const char* path);
void blocks_free();
void* blocks_get_block(int pnum);
void* get_blocks_bitmap();
void* get_inode_bitmap();
int alloc_block();
void free_block(int pnum);

#endif
