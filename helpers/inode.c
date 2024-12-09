// inode.c

#include "inode.h"
#include "blocks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INODES 1024

static inode_t inode_table[MAX_INODES];
static int inode_bitmap[MAX_INODES] = {0}; // 0 = free, 1 = allocated

void print_inode(inode_t *node) {
    if (!node) {
        printf("Invalid inode.\n");
        return;
    }
    printf("Inode Details:\n");
    printf("Refs: %d\n", node->refs);
    printf("Mode: %d\n", node->mode);
    printf("Size: %d bytes\n", node->size);
    printf("Block: %d\n", node->block);
}

inode_t *get_inode(int inum) {
    if (inum < 0 || inum >= MAX_INODES || !inode_bitmap[inum]) {
        return NULL;
    }
    return &inode_table[inum];
}

int alloc_inode() {
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_bitmap[i] == 0) {
            inode_bitmap[i] = 1;
            memset(&inode_table[i], 0, sizeof(inode_t));
            inode_table[i].refs = 1;
            return i;
        }
    }
    return -1; // No available inode
}

void free_inode(int inum) {
    if (inum < 0 || inum >= MAX_INODES || !inode_bitmap[inum]) {
        return;
    }
    inode_bitmap[inum] = 0;
    memset(&inode_table[inum], 0, sizeof(inode_t));
}

int grow_inode(inode_t *node, int size) {
    if (!node) return -1;
    if (size <= node->size) return 0;

    int additional_size = size - node->size;
    int additional_blocks = (additional_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int new_block = allocate_block(additional_blocks);
    if (new_block == -1) return -1;

    node->block = new_block;
    node->size = size;
    return 0;
}

int shrink_inode(inode_t *node, int size) {
    if (!node) return -1;
    if (size >= node->size) return 0;

    int reduced_size = node->size - size;
    int blocks_to_free = (reduced_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    for (int i = 0; i < blocks_to_free; i++) {
        free_block(node->block + i);
    }
    
    node->size = size;
    return 0;
}

int inode_get_bnum(inode_t *node, int file_bnum) {
    if (!node) return -1;
    // Assuming single indirect blocks for simplicity
    if (file_bnum < 0) return -1;
    return node->block + file_bnum;
}