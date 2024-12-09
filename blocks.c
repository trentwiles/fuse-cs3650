#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>

// our c header files
#include "blocks.h"
#include "util.h"
#include "bitmap.h"


static int   blocks_fd   = -1;
static void* blocks_base =  0;

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

void blocks_free()
{
    int rv = munmap(blocks_base, NUFS_SIZE);
    assert(rv == 0);
}

void* blocks_get_block(int pnum)
{
    return blocks_base + 4096 * pnum;
}

void* get_blocks_bitmap()
{
    return blocks_get_block(0);
}

void*
get_inode_bitmap()
{
    uint8_t* blockz = blocks_get_block(0);
    return (void*)(blockz + 32);
}

int alloc_blocks()
{
    void* pbm = get_blocks_bitmap();

    for (int ii = 1; ii < BLOCK_COUNT; ++ii) {
        if (!bitmap_get(pbm, ii)) {
            bitmap_put(pbm, ii, 1);
            return ii;
        }
    }

    return -1;
}

void free_blocks(int pnum)
{
    void* pbm = get_blocks_bitmap();
    bitmap_put(pbm, pnum, 0);
}

