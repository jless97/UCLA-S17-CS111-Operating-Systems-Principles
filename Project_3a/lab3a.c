////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less & Yun Xu
// EMAIL: jaless1997@gmail.com &
// ID: 404-640-158 &
// Project 3a: lab3a.c

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#include "ext2_fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Defines ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Super block located at byte offset 1024 from beginning of volume
#define SUPER_BLOCK_OFFSET 1024
// Super block size
#define SUPER_BLOCK_SIZE 1024
// Super block csv fields: only concerned with first 100 or so bytes
#define SUPER_BLOCK_BUFFER_SIZE 128
// Block group descriptor table size
#define DESCRIPTOR_TABLE_SIZE 32
// Block group descriptor table buffer size
#define DESCRIPTOR_TABLE_BUFFER_SIZE 32

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Globals ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/* Programmable Variables */
// File descriptor for the image file provided
int image_fd;
// Buffer to acquire information for the super block
char super_block_buf[SUPER_BLOCK_BUFFER_SIZE];
// Buffer to acquire information for the block group descriptor table
char descriptor_table_buf[DESCRIPTOR_TABLE_BUFFER_SIZE];
// Number of block groups
int num_groups;

/* Flags */


/* Structs */
// Super block struct to store the super block information
struct ext2_super_block super_block;
// Block group descriptor table struct
struct ext2_group_desc group_descriptor_table;
// Block group struct format
struct p3_block_group {
    __u32 group_id;             /* Block group number */
    __u32 g_blocks_count;       /* Blocks count */
    __u32 g_inodes_count;       /* Inodes count */
    __u32 g_free_blocks_count;  /* Free blocks count */
    __u32 g_free_inodes_count;  /* Free inodes count */
    __u32 g_block_bitmap;       /* Blocks bitmap block */
    __u32 g_inode_bitmap;       /* Inodes bitmap block */
    __u32 g_inode_table;        /* Inodes table block */
};
// Block group array
struct p3_block_group *block_group;

/* Arrays */


////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Prototypes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Print usage upon unrecognized arguments
void print_usage(void);
// Parse command-line options
void parser(int argc, char *argv[]);
// Acquire super block information
void getSuperBlock(struct ext2_super_block *block);
// Print super block csv record
void printSuperBlockCSVRecord(void);
// Acquire block group information
void getBlockGroup(struct p3_block_group *group);
// Print block group csv record
void printBlockGroupCSVRecord(void);
// Acquire free block entries
void getFreeBlock(void);
// Print free block entires csv record
void printFreeBlockCSVRecord(int num_block);
// Acquire free inode entries
void getFreeInode(void);
// Print free inode entrires csv record
void printFreeInodeCSVRecord(int num_block);

// Acquire inode summary

// Print inode summary csv record
void printInodeSummaryCSVRecord(void);

// Acquire directory entries

// Print directory entries csv record
void printDirectoryCSVRecord(void);

// Acquire indirect block references

// Print indirect block references csv record
void printIndirectCSVRecord(void);

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
print_usage(void) {
    printf("Usage: ./lab3a [Image File]\n");
}

void
parser(int argc, char * argv[]) {
    if (argc != 2) {
        print_usage();
        exit(EXIT_FAILURE);
    }
    
    image_fd = open(argv[1], O_RDONLY);
    if (image_fd < 0) {
        fprintf(stderr, "Error opening image file.\n");
        exit(EXIT_FAILURE);
    }
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Super Block ////////////////////////////
////////////////////////////////////////////////////////////////////////////
void
getSuperBlock(struct ext2_super_block *block) {
    // Initialize the buffer to read in for super block
    memset(super_block_buf, 0, SUPER_BLOCK_BUFFER_SIZE);
    
    ssize_t nread = pread(image_fd, super_block_buf, SUPER_BLOCK_BUFFER_SIZE, SUPER_BLOCK_OFFSET);
    if (nread < 0) {
        fprintf(stderr, "Error reading super block info from image file.\n");
        exit(EXIT_FAILURE);
    }
    
    // Total number of blocks
    memcpy(&block->s_blocks_count, super_block_buf + 4, 4);
    
    // Total number of inodes
    memcpy(&block->s_inodes_count, super_block_buf, 4);
    
    // Size of block
    memcpy(&block->s_log_block_size, super_block_buf + 24, 4);
    block->s_log_block_size = 1024 << block->s_log_block_size;
    
    // Size of inode
    memcpy(&block->s_inode_size, super_block_buf + 88, 2);
    
    // Number of blocks per group
    memcpy(&block->s_blocks_per_group, super_block_buf + 32, 4);
    
    // Number of inodes per group
    memcpy(&block->s_inodes_per_group, super_block_buf + 40, 4);
    
    // Index to first non-reserved inode
    memcpy(&block->s_first_ino, super_block_buf + 88, 4);
}

void
printSuperBlockCSVRecord(void) {
    // Print to STDOUT super block CSV record
    fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d\n", "SUPERBLOCK", super_block.s_blocks_count, super_block.s_inodes_count, super_block.s_log_block_size, super_block.s_inode_size, super_block.s_blocks_per_group, super_block.s_inodes_per_group, super_block.s_first_ino);
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Block Group ////////////////////////////
////////////////////////////////////////////////////////////////////////////
void
getBlockGroup(struct p3_block_group *group) {
    // Get the number of block groups
    num_groups = (int)ceil((double) super_block.s_blocks_count / (double) super_block.s_blocks_per_group);
    
    // If there are some leftover blocks for last block group
    int leftover_blocks = super_block.s_blocks_count % super_block.s_blocks_per_group;
    
    // Create block group array
    block_group = (struct p3_block_group *) malloc(sizeof(struct p3_block_group) * num_groups);
    
    ssize_t nread;
    int i, last_group = num_groups - 1;
    for (i = 0; i < num_groups; i++) {
        // Initialize the buffer to read in for descriptor table
        memset(descriptor_table_buf, 0, DESCRIPTOR_TABLE_BUFFER_SIZE);
        
        nread = pread(image_fd, descriptor_table_buf, DESCRIPTOR_TABLE_BUFFER_SIZE, SUPER_BLOCK_SIZE + (i * DESCRIPTOR_TABLE_SIZE));
        if (nread < 0) {
            fprintf(stderr, "Error reading block group descriptor table info from image file.\n");
            exit(EXIT_FAILURE);
        }
        
        // Group ID number
        group[i].group_id = i;
        
        // Total number of blocks (if there are leftover blocks, last group has the leftover)
        if ((i = last_group) && (leftover_blocks == 0)) {
            group[i].g_blocks_count = super_block.s_blocks_per_group;
        }
        else {
            group[i].g_blocks_count = leftover_blocks;
        }
        
        // Total number of inodes
        group[i].g_inodes_count = super_block.s_inodes_per_group;
        
        // Total number of free blocks
        memcpy(&group[i].g_free_blocks_count, descriptor_table_buf, 4);
        
        // Total number of free inodes
        memcpy(&group[i].g_free_inodes_count, descriptor_table_buf + 4, 4);
        
        // Block number of free block bitmap
        memcpy(&group[i].g_block_bitmap, descriptor_table_buf + 8, 2);
        
        // Block number of free inode bitmap
        memcpy(&group[i].g_inode_bitmap, descriptor_table_buf + 12, 2);
        
        // Block number of first block of inodes
        memcpy(&group[i].g_inode_table, descriptor_table_buf + 14, 4);
    }
}

void
printBlockGroupCSVRecord(void) {
    // Print to STDOUT block group CSV record
    int i;
    for (i = 0; i < num_groups; i++) {
        fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d,%d\n", "GROUP", block_group[i].group_id, block_group[i].g_blocks_count, block_group[i].g_inodes_count, block_group[i].g_free_blocks_count, block_group[i].g_free_inodes_count, block_group[i].g_block_bitmap, block_group[i].g_inode_bitmap, block_group[i].g_inode_table);
    }
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Free Block Entries /////////////////////////
////////////////////////////////////////////////////////////////////////////
void
getFreeBlock(void) {
    // Block size for the free block bitmap
    int block_size = super_block.s_log_block_size;
    
    // Total number of bits in bitmap
    int bit_size = block_size * 8;
    
    // Variable to keep track of which block is free/allocated
    int num_block = 0;
    
    // Variable to hold the bitmap block
    long block_bitmap_buf;
    
    ssize_t nread;
    int i, bitmask = 1;
    for (i = 0; i < num_groups; i++) {
        nread = pread(image_fd, &block_bitmap_buf, block_size, block_group[i].g_block_bitmap);
        if (nread < 0) {
            fprintf(stderr, "Error reading free block bitmap info from image file.\n");
            exit(EXIT_FAILURE);
        }
        // While there are still bits left to read, continue
        while (bit_size != 0) {
            // If the bit being checked is a 0, then the corresponding block is free
            if ((block_bitmap_buf && bitmask) == 0) {
                printFreeBlockCSVRecord(num_block);
            }
            // Shift the bits to the right by 1 to read next bit
            block_bitmap_buf = block_bitmap_buf >> 1;
            // Increment the block number
            num_block++;
            // Decrement the bit size as a bit was just read
            bit_size--;
        }
        // Reset variables
        bit_size = block_size * 8;
        num_block = 0;
        block_bitmap_buf = 0;
    }
}

void
printFreeBlockCSVRecord(int num_block) {
    // Print to STDOUT free block CSV record
    fprintf(stdout, "%s,%d\n", "BFREE", num_block);
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Free Inode Entries /////////////////////////
////////////////////////////////////////////////////////////////////////////
void
getFreeInode(void) {
    // Block size for the free block bitmap
    int block_size = super_block.s_log_block_size;
    
    // Total number of bits in bitmap
    int bit_size = block_size * 8;
    
    // Variable to keep track of which block is free/allocated
    int num_block = 0;
    
    // Variable to hold the bitmap block
    long block_bitmap_buf;
    
    ssize_t nread;
    int i, bitmask = 1;
    for (i = 0; i < num_groups; i++) {
        nread = pread(image_fd, &block_bitmap_buf, block_size, block_group[i].g_inode_bitmap);
        if (nread < 0) {
            fprintf(stderr, "Error reading free inode bitmap info from image file.\n");
            exit(EXIT_FAILURE);
        }
        // While there are still bits left to read, continue
        while (bit_size != 0) {
            // If the bit being checked is a 0, then the corresponding block is free
            if ((block_bitmap_buf && bitmask) == 0) {
                printFreeBlockCSVRecord(num_block);
            }
            // Shift the bits to the right by 1 to read next bit
            block_bitmap_buf = block_bitmap_buf >> 1;
            // Increment the block number
            num_block++;
            // Decrement the bit size as a bit was just read
            bit_size--;
        }
        // Reset variables
        bit_size = block_size * 8;
        num_block = 0;
        block_bitmap_buf = 0;
    }
}

void
printFreeInodeCSVRecord(int num_block) {
    // Print to STDOUT free inodes CSV record
    fprintf(stdout, "%s,%d\n", "IFREE", num_block);
}

////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Inode Summary ////////////////////////////
////////////////////////////////////////////////////////////////////////////

void
printInodeSummaryCSVRecord(void) {
    // Print to STDOUT inode summary CSV record
    
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Directory Entries /////////////////////////
////////////////////////////////////////////////////////////////////////////

void
printDirectoryCSVRecord(void) {
    // Print to STDOUT directory entries CSV record
    
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////// Indirect Block References /////////////////////
////////////////////////////////////////////////////////////////////////////

void
printIndirectCSVRecord(void) {
    // Print to STDOUT indirect block references CSV record
    
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main (int argc, char *argv[])
{
    // Parse command-line options
    parser(argc, argv);
    
    // Get super block information and print it to STDOUT
    getSuperBlock(&super_block);
    printSuperBlockCSVRecord();
    
    // Get block group information and print it to STDOUT
    getBlockGroup(block_group);
    printBlockGroupCSVRecord();
    
    // If success
    exit(EXIT_SUCCESS);
}
