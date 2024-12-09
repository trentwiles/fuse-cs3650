#include <stdlib.h>
#include <time.h>
#include <stdio.h>
// for unit8_t
#include <stdint.h>

#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

// print off some metadata about the inode 
void print_inode(inode_t* node) {
    if (node == NULL) {
        printf("==== INODE DEBUGGING ====\n");
        printf("Null inode pointer provided.\n");
        return;
    }

    printf("==== INODE DEBUGGING ====\n");
    printf("Inode pointer: %p\n", (void*)node);
    printf("Type & Permissions: %d\n", node->mode);
    printf("Reference Count: %d\n", node->refs);
    printf("Size (bytes): %d\n", node->size);
    printf("Indirect Pointer Count: %d\n", node->iptr);
    printf("Direct Pointers: %d, %d\n", node->ptrs[0], node->ptrs[1]);
}

// grabs the pointer to an inode structure
// in the form of inode*
inode_t* get_inode(int inum) {
    inode_t* inodes = get_inode_bitmap() + 32;
    return &inodes[inum];
}

int alloc_inode() {
    uint8_t* inode_bitmap = get_inode_bitmap();
    if (inode_bitmap == NULL) {
        // If we cannot retrieve the inode bitmap, return an error code
        return -1;
    }

    int nodenum = -1;
    for (int i = 0; i < 256; i++) {
        if (!bitmap_get(inode_bitmap, i)) {
            bitmap_put(inode_bitmap, i, 1);
            nodenum = i;
            break;
        }
    }

    // If we did not find a free inode, return an error
    if (nodenum == -1) {
        return -1;
    }

    inode_t* new_node = get_inode(nodenum);
    if (new_node == NULL) {
        // If we fail to get the inode for some reason, revert bitmap changes and return an error
        bitmap_put(inode_bitmap, nodenum, 0);
        return -1;
    }

    // Initialize the new inode fields
    new_node->refs = 1;
    new_node->size = 0;
    new_node->mode = 0;
    
    // Allocate a block for the inode
    int block_num = alloc_block();
    if (block_num < 0) {
        // Failed to allocate a block, revert bitmap changes and return an error
        bitmap_put(inode_bitmap, nodenum, 0);
        return -1;
    }
    new_node->ptrs[0] = block_num;
    new_node->ptrs[1] = 0;
    new_node->iptr = 0; // Assuming iptr should be initialized as well

    return nodenum;
}


// destroys the inode (marks as free)
// employs our helper methods, shrink_inode and bitmap_put
// shrink inode is defined below
void free_inode(int inum) {
    // grab the inode
    inode_t* node = get_inode(inum);
    void* bitmap = get_inode_bitmap();

    // process of freeing inode resources (shrink + free)
    shrink_inode(node, 0);
    if (node->ptrs[0] != 0) {
        free_block(node->ptrs[0]);
    }

    bitmap_put(bitmap, inum, 0);
}

// grows the inode, if size gets too big, it allocates a new block if possible
int grow_inode(inode_t* node, int size) {
    for (int i = (node->size / 4096) + 1; i <= size / 4096; i ++) {
        if (i < nptrs) {
            node->ptrs[i] = alloc_block();
        } else {
            if (node->iptr == 0) { //get a page if we don't have one
                node->iptr = alloc_block();
            }
            int* iptrs = blocks_get_block(node->iptr); //retrieve memory loc.
            iptrs[i - nptrs] = alloc_block();
        }
    }
    node->size = size;
    return 0;
}

// shrinks an inode size and deallocates pages if we've freed them up
int shrink_inode(inode_t* node, int size) {
    for (int i = (node->size / 4096); i > size / 4096; i --) {
        if (i < nptrs) { //we're in direct ptrs
            free_block(node->ptrs[i]); //free the page
            node->ptrs[i] = 0;
        } else { //need to use indirect
            int* iptrs = blocks_get_block(node->iptr); //retrieve memory loc.
            free_block(iptrs[i - nptrs]); //free the single page
            iptrs[i-nptrs] = 0;

            if (i == nptrs) { //if that was the last thing on the page
                free_block(node->iptr); //we don't need it anymore
                node->iptr = 0;
            }
        }
    } 
    node->size = size;
    return 0;  
}

// gets the page number for the inode
int inode_get_pnum(inode_t* node, int fpn) {
    int blocknum = fpn / 4096;
    if (blocknum < nptrs) {
        return node->ptrs[blocknum];
    } else {
        int* iptrs = blocks_get_block(node->iptr);
        return iptrs[blocknum-nptrs];
    }
}

void decrease_refs(int inum)
{
    inode_t* node = get_inode(inum);
    node->refs = node->refs - 1;
    if (node->refs < 1) {
        free_inode(inum);
    }
}
