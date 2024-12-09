// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

// max dir name size, CHANGE LATER
#define DIR_NAME 48

#include "slist.h"
#include "blocks.h"
#include "inode.h"

typedef struct dirent {
    char name[DIR_NAME];// The name of the directory
    int  inum; // the number of the inode in the inode table?
    char used; // is this directory entry being used?
    char _reserved[11]; // round it out to 64B
} dirent;

// not sure what this does
void directory_init(); 
int directory_lookup(inode* dd, const char* name);
int tree_lookup(const char* path);
int directory_put(inode* dd, const char* name, int inum);
int directory_delete(inode* dd, const char* name);
slist_t* directory_list(const char* path);
void print_directory(inode* dd);

#endif

