// inode implementation

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "inode.h"
#include "pages.h"
#include "bitmap.h"

/* Definition of the inode structure:

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[2]; // direct pointers
    int iptr; // single indirect pointer
    time_t atim;
    time_t ctim;
    time_t mtim;
} inode; */

void 
print_inode(inode* node) {
    printf("inode located at %p:\n", node);
    printf("Reference count: %d\n", node->refs);
    printf("Node Permission + Type: %d\n", node->mode);
    printf("Node size in bytes: %d\n", node->size);
    printf("Node direct pointers: %d, %d\n", node->ptrs[0], node->ptrs[1]);
    printf("Node indirect pointer: %d\n", node->iptr);
}

inode* 
get_inode(int inum) {
    inode* inodes = get_inode_bitmap() + 32;
    return &inodes[inum];
}

// take a look at the malloc implementation
int 
alloc_inode() {
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
    new_node->ptrs[0] = alloc_page();
    
    time_t curtime = time(NULL);
    new_node->ctim = curtime;
    new_node->atim = curtime;
    new_node->mtim = curtime;

    return nodenum;
}

// marks the inode as free in the bitmap and then clears the pointer locations
void 
free_inode(int inum) {
    printf("+ free_inode(%d)\n", inum);
    inode* node = get_inode(inum);
    void* bmp = get_inode_bitmap(); 
    shrink_inode(node, 0);
    free_page(node->ptrs[0]);
    bitmap_put(bmp, inum, 0);
}

// grows the inode, if size gets too big, it allocates a new page if possible
int grow_inode(inode* node, int size) {
    for (int i = (node->size / 4096) + 1; i <= size / 4096; i ++) {
        if (i < nptrs) { //we can use direct ptrs
            node->ptrs[i] = alloc_page(); //alloc a page
        } else { //need to use indirect
            if (node->iptr == 0) { //get a page if we don't have one
                node->iptr = alloc_page();
            }
            int* iptrs = pages_get_page(node->iptr); //retrieve memory loc.
            iptrs[i - nptrs] = alloc_page(); //add another page
        }
    }
    node->size = size;
    return 0;
}

// shrinks an inode size and deallocates pages if we've freed them up
int shrink_inode(inode* node, int size) {
    for (int i = (node->size / 4096); i > size / 4096; i --) {
        if (i < nptrs) { //we're in direct ptrs
            free_page(node->ptrs[i]); //free the page
            node->ptrs[i] = 0;
        } else { //need to use indirect
            int* iptrs = pages_get_page(node->iptr); //retrieve memory loc.
            free_page(iptrs[i - nptrs]); //free the single page
            iptrs[i-nptrs] = 0;

            if (i == nptrs) { //if that was the last thing on the page
                free_page(node->iptr); //we don't need it anymore
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
        int* iptrs = pages_get_page(node->iptr);
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
