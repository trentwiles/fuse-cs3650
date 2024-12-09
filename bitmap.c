// Implementation of bitmap.h

#include "bitmap.h"
#include <stdio.h>

// gets the value at but ii
int bitmap_get(void* bm, int ii) {
    int jj = ii % 8;

    return ((char*)bm)[ii / 8] >> jj & 1;
}

// puts a value at the specified bit in the bitmap
void bitmap_put(void* bm, int ii, int vv) {
    bitmap_print(bm, ii);
    int bit = (vv) ? 1 : 0;
    char* bitmap = (char*)bm;
    if (bitmap_get(bm, ii) == bit) {
        return;
    }
    else {
        bitmap[(ii / 8)] ^= 1 << ii % 8; 
    }
    bitmap_print(bm, ii);
}

// debug statement
void bitmap_print(void* bm, int size) {
    for (int ii = 0; ii < size * 4; ++ii) {
        printf("%d", bitmap_get(bm, ii));
    }
}
