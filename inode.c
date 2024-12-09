#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// for unit8_t
#include <stdint.h>

#include "bitmap.h"
#include "blocks.h"
#include "inode.h"

// print off some metadata about the inode
void print_inode(inode_t *node) {
  if (node == NULL) {
    printf("==== INODE DEBUGGING ====\n");
    printf("Null inode pointer provided.\n");
    return;
  }

  printf("==== INODE DEBUGGING ====\n");
  printf("Inode pointer: %p\n", (void *)node);
  printf("Type & Permissions: %d\n", node->mode);
  printf("Reference Count: %d\n", node->refs);
  printf("Size (bytes): %d\n", node->size);
  printf("Indirect Pointer Count: %d\n", node->iptr);
  printf("Direct Pointers: %d, %d\n", node->ptrs[0], node->ptrs[1]);
}

// grabs the pointer to an inode structure
// in the form of inode*
inode_t *get_inode(int inum) {
  inode_t *inodes = get_inode_bitmap() + 32;
  return &inodes[inum];
}

int alloc_inode() {
  uint8_t *inode_bitmap = get_inode_bitmap();

  // erorr if inode bitmap wasn't found
  if (inode_bitmap == NULL) {
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

  // -1 indicts a free inode can't be found
  if (nodenum == -1) {
    return -1;
  }

  inode_t *the_new_node = get_inode(nodenum);
  if (the_new_node == NULL) {
    // If we fail to get the inode for some reason, revert bitmap changes and
    // return an error
    bitmap_put(inode_bitmap, nodenum, 0);
    return -1;
  }

  // Initialize the new inode fields
  the_new_node->refs = 1;
  the_new_node->size = 0;
  the_new_node->mode = 0;

  // Allocate a block for the inode
  int block_num = alloc_block();
  if (block_num < 0) {
    // for whatever reason we couldn't allocate a block
    bitmap_put(inode_bitmap, nodenum, 0);

    // but really, we should never reach this!!
    return -1;
  }

  the_new_node->ptrs[0] = block_num;
  the_new_node->ptrs[1] = 0;
  the_new_node->iptr = 0;

  return nodenum;
}

void free_inode(int inum) {

  // find inum as requested
  inode_t *node = get_inode(inum);

  if (node == NULL) {
    // if null, exit
    return;
  }

  // find the respective bitmap
  uint8_t *inode_bitmap = get_inode_bitmap();
  if (inode_bitmap == NULL) {
    // if bitmap is null, exit
    return;
  }

  // free the blocks used by the inode
  shrink_inode(node, 0);

  if (node->ptrs[0] != 0) {
    free_block(node->ptrs[0]);
  }

  // once done mark as free!!!
  bitmap_put(inode_bitmap, inum, 0);
}

int grow_inode(inode_t *node, int new_size) {
  if (node == NULL) {
    return -1;
  }

  // Calculate how many blocks we currently have and how many we need
  int current_block_count = node->size / 4096;
  int needed_block_count = new_size / 4096;

  // Loop from the first new block that we need to allocate up to the required
  // number of blocks
  for (int i = current_block_count + 1; i <= needed_block_count; i++) {
    if (i < nptrs) {

      int block_num = alloc_block();
      if (block_num < 0) {
        // couldn't allocate so exit
        return -1;
      }
      node->ptrs[i] = block_num;
    } else {
      // no direct pointers, switch to indirect pointers
      if (node->iptr == 0) {

        // allocate indirect pointer
        // (only if needed)
        int iptr_block = alloc_block();
        if (iptr_block < 0) {
          return -1;
        }
        node->iptr = iptr_block;
      }

      // grab the indirect block array
      int *indirect_ptrs = blocks_get_block(node->iptr);
      if (indirect_ptrs == NULL) {
        return -1;
      }

      // allocate new block, for the pointer
      int block_num = alloc_block();
      if (block_num < 0) {
        // failed to allocate so exit...
        return -1;
      }
      indirect_ptrs[i - nptrs] = block_num;
    }
  }

  // update inode size following what we've done
  node->size = new_size;
  return 0;
}

// shrinks an inode size and deallocates pages if we've freed them up
int shrink_inode(inode_t *node, int size) {
  for (int i = (node->size / 4096); i > size / 4096; i--) {
    if (i < nptrs) {             // we're in direct ptrs
      free_block(node->ptrs[i]); // free the page
      node->ptrs[i] = 0;
    } else {                                     // need to use indirect
      int *iptrs = blocks_get_block(node->iptr); // retrieve memory loc.
      free_block(iptrs[i - nptrs]);              // free the single page
      iptrs[i - nptrs] = 0;

      if (i == nptrs) {         // if that was the last thing on the page
        free_block(node->iptr); // we don't need it anymore
        node->iptr = 0;
      }
    }
  }
  node->size = size;
  return 0;
}

// gets the page number for the inode
int inode_get_bnum(inode_t *node, int file_bnum) {
  int blocknum = file_bnum / 4096;
  if (blocknum < nptrs) {
    return node->ptrs[blocknum];
  } else {
    int *iptrs = blocks_get_block(node->iptr);
    return iptrs[blocknum - nptrs];
  }
}

void decrease_refs(int inum) {
  inode_t *node = get_inode(inum);
  node->refs = node->refs - 1;
  if (node->refs < 1) {
    free_inode(inum);
  }
}
