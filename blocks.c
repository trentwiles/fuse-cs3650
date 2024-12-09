/**
 * @file blocks.c
 * @author CS3650 staff
 *
 * Implementatino of a block-based abstraction over a disk image file.
 */
#define _GNU_SOURCE
#include <string.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


// our c header files
#include "blocks.h"
#include "bitmap.h"

// we removed ferds's block count in leiu of define

static int   blocks_fd   = -1;
static void* blocks_base =  0;

// Get the number of blocks needed to store the given number of bytes.
int bytes_to_blocks(int bytes) {
  int quo = bytes / BLOCK_SIZE;
  int rem = bytes % BLOCK_SIZE;
  if (rem == 0) {
    return quo;
  } else {
    return quo + 1;
  }
}

// Load and initialize the given disk image.
void blocks_init(const char* path)
{
    blocks_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(blocks_fd != -1);

    int rv = ftruncate(blocks_fd, NUFS_SIZE);
    assert(rv == 0);

    blocks_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, blocks_fd, 0);
    assert(blocks_base != MAP_FAILED);

    void* pbm = get_blocks_bitmap();
    bitmap_put(pbm, 0, 1);
}

// Close the disk image.
void blocks_free()
{
    int rv = munmap(blocks_base, NUFS_SIZE);
    assert(rv == 0);
}

// Get the given block, returning a pointer to its start.
void* blocks_get_block(int pnum)
{
    return blocks_base + 4096 * pnum;
}

// Return a pointer to the beginning of the block bitmap.
// The size is BLOCK_BITMAP_SIZE bytes.
void* get_blocks_bitmap()
{
    return blocks_get_block(0);
}

// Return a pointer to the beginning of the inode table bitmap.
void* get_inode_bitmap()
{
    uint8_t* blockz = blocks_get_block(0);
    
    // The inode bitmap is stored immediately after the block bitmap
    return (void*)(blockz + 32);
}

// Allocate a new block and return its index.
int alloc_block()
{
    void* bbm = get_blocks_bitmap();
    
    // more or less the starter code function
    for (int ii = 1; ii < BLOCK_COUNT; ++ii) {
        if (!bitmap_get(bbm, ii)) {
            bitmap_put(bbm, ii, 1);
            return ii;
        }
    }

    return -1;
}

// Deallocate the block with the given index.
void free_block(int pnum)
{
    void* pbm = get_blocks_bitmap();
    bitmap_put(pbm, pnum, 0);
}

