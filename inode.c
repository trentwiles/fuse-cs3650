#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

// debugging function to print off the entire 
void print_inode(inode* node) {
    printf("==== INODE DEBUGGING ====");
    printf("inode located at %p:\n", node);
    printf("Node Type & Permission: %d\n", node->mode);
    printf("Reference count: %d\n", node->refs);
    printf("Node size (B): %d\n", node->size);
    printf("Node indirect pointer count: %d\n", node->iptr);
    printf("Node direct pointer count: %d, %d\n", node->ptrs[0], node->ptrs[1]);
}

// grabs the pointer to an inode structure
// in the form of inode*
inode* get_inode(int inum) {
    inode* inodes = get_inode_bitmap() + 32;
    return &inodes[inum];
}

// creates a new inode, and provides it's fields with default values
int alloc_inode() {
    int nodenum;
    for (int ii = 0; ii < 256; ++ii) {
        if (!bitmap_get(get_inode_bitmap(), ii)) {
            bitmap_put(get_inode_bitmap(), ii, 1);
            nodenum = ii;
            break;
        }
    }
    inode* new_node = get_inode(nodenum);
    new_node->refs = 1;
    new_node->size = 0;
    new_node->mode = 0;
    new_node->ptrs[0] = alloc_blocks();
    
    time_t curtime = time(NULL);
    new_node->ctim = curtime;
    new_node->atim = curtime;
    new_node->mtim = curtime;

    return nodenum;
}

// destroys the inode (marks as free)
// employs our helper methods, shrink_inode and bitmap_put
// shrink inode is defined below
void free_inode(int inum) {
    // grab the inode
    inode* node = get_inode(inum);
    void* bitmap = get_inode_bitmap();

    // process of freeing inode resources (shrink + free)
    shrink_inode(node, 0);
    if (node->ptrs[0] != 0) {
        free_blocks(node->ptrs[0]);
    }

    bitmap_put(bitmap, inum, 0);
}

// grows the inode, if size gets too big, it allocates a new block if possible
int grow_inode(inode* node, int size) {
    for (int i = (node->size / 4096) + 1; i <= size / 4096; i ++) {
        if (i < nptrs) {
            node->ptrs[i] = alloc_blocks();
        } else {
            if (node->iptr == 0) { //get a page if we don't have one
                node->iptr = alloc_blocks();
            }
            int* iptrs = blocks_get_block(node->iptr); //retrieve memory loc.
            iptrs[i - nptrs] = alloc_blocks();
        }
    }
    node->size = size;
    return 0;
}

// shrinks an inode size and deallocates pages if we've freed them up
int shrink_inode(inode* node, int size) {
    for (int i = (node->size / 4096); i > size / 4096; i --) {
        if (i < nptrs) { //we're in direct ptrs
            free_blocks(node->ptrs[i]); //free the page
            node->ptrs[i] = 0;
        } else { //need to use indirect
            int* iptrs = blocks_get_block(node->iptr); //retrieve memory loc.
            free_blocks(iptrs[i - nptrs]); //free the single page
            iptrs[i-nptrs] = 0;

            if (i == nptrs) { //if that was the last thing on the page
                free_blocks(node->iptr); //we don't need it anymore
                node->iptr = 0;
            }
        }
    } 
    node->size = size;
    return 0;  
}

// gets the page number for the inode
int inode_get_pnum(inode* node, int fpn) {
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
    inode* node = get_inode(inum);
    node->refs = node->refs - 1;
    if (node->refs < 1) {
        free_inode(inum);
    }
}
