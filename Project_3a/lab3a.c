////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// NAME: Jason Less & Yun Xu
// EMAIL: jaless1997@gmail.com & x_one_u@yahoo.com
// ID: 404-640-158 & 304-635-157
// Project 3a: lab3a.c

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include "ext2_fs.h"

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
// Size of inode
#define INODE_SIZE 128
// Inode table buffer size
#define INODE_BUFFER_SIZE 128

// File type specifiers
#define EXT2_S_IFREG 32768
#define EXT2_S_IFDIR 16384
#define EXT2_S_IFLNK 40960
// Mode type specifiers
#define EXT2_S_IRUSR 256
#define EXT2_S_IWUSR 128
#define EXT2_S_IXUSR 64
#define EXT2_S_IRGRP 32
#define EXT2_S_IWGRP 16
#define EXT2_S_IXGRP 8
#define EXT2_S_IROTH 4
#define EXT2_S_IWOTH 2
#define EXT2_S_IXOTH 1

// Time field length
#define TIME_BUFFER 18

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
// Buffer to acquire information for the inodes
char inode_buf[INODE_BUFFER_SIZE];
// Number of block groups
int num_groups;
// Number of inodes per group
int num_inodes;

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
// Inode table information array
struct ext2_inode **inode_table;

/* Arrays */
// Array to keep of which inodes are allocated/free for each group
int **inode_array;

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
void getBlockGroup(void);
// Print block group csv record
void printBlockGroupCSVRecord(void);
// Acquire free block entries
void getFreeBlock(void);
// Print free block entires csv record
void printFreeBlockCSVRecord(int num_block);
// Acquire free inode entries
void getFreeInode();
// Print free inode entrires csv record
void printFreeInodeCSVRecord(int num_block);
// Acquire inode summary
void getInodeSummary(void);
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
getBlockGroup(void) {
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
        block_group[i].group_id = i;
        
        // Total number of blocks (if there are leftover blocks, last group has the leftover)
        if ((i = last_group) && (leftover_blocks == 0)) {
            block_group[i].g_blocks_count = super_block.s_blocks_per_group;
        }
        else {
            block_group[i].g_blocks_count = leftover_blocks;
        }
        
        // Total number of inodes
        block_group[i].g_inodes_count = super_block.s_inodes_per_group;
        
        // Total number of free blocks
        memcpy(&block_group[i].g_free_blocks_count, descriptor_table_buf, 4);
        
        // Total number of free inodes
        memcpy(&block_group[i].g_free_inodes_count, descriptor_table_buf + 4, 4);
        
        // Block number of free block bitmap
        memcpy(&block_group[i].g_block_bitmap, descriptor_table_buf + 8, 2);
        
        // Block number of free inode bitmap
        memcpy(&block_group[i].g_inode_bitmap, descriptor_table_buf + 12, 2);
        
        // Block number of first block of inodes
        memcpy(&block_group[i].g_inode_table, descriptor_table_buf + 14, 4);
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
    // Variable to hold the bitmap block (reading 1 byte at a time)
    __u8 block_bitmap_buf;
    
    ssize_t nread;
    int i, j, bit_size = 8, block_size = super_block.s_log_block_size, num_block = 0, bitmask = 1;
    for (i = 0; i < num_groups; i++) {
        j = 0;
        while (j != block_size) {
            block_bitmap_buf = 0;
            nread = pread(image_fd, &block_bitmap_buf, 1, block_group[i].g_block_bitmap + j);
            if (nread < 0) {
                fprintf(stderr, "Error reading free block bitmap info from image file.\n");
                exit(EXIT_FAILURE);
            }
            // No more bytes to read from bitmap
            if (nread == 0) {
                bit_size = 0;
            }
            else {
                bit_size = 8;
            }
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
            j++;
        }
        // Reset block number for next block group
        num_block = 0;
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
    int i, j, bit_size = 8, block_size = super_block.s_log_block_size, num_block = 0, bitmask = 1;
    
    // Initialize the inode free/allocated array
    num_inodes = super_block.s_inodes_per_group;
    inode_array = (int **) malloc(sizeof(int *) * num_groups);
    for (i = 0; i < num_groups; i++) {
        inode_array[i] = (int *) malloc(sizeof(int) * num_inodes);
    }
    
    // Variable to hold the bitmap block (reading 1 byte at a time)
    __u8 inode_bitmap_buf;
    
    ssize_t nread;
    for (i = 0; i < num_groups; i++) {
        j = 0;
        while (j != num_inodes) {
            inode_bitmap_buf = 0;
            nread = pread(image_fd, &inode_bitmap_buf, 1, block_group[i].g_inode_bitmap + j);
            if (nread < 0) {
                fprintf(stderr, "Error reading free inode bitmap info from image file.\n");
                exit(EXIT_FAILURE);
            }
            // No more bytes to read from bitmap
            if (nread == 0) {
                bit_size = 0;
            }
            else {
                bit_size = 8;
            }
            while (bit_size != 0) {
                // If the bit being checked is a 0, then the corresponding block is free
                if ((inode_bitmap_buf && bitmask) == 0) {
                    // Set portion of inode array to note that inode block is free
                    inode_array[i][j] = 0;
                    
                    printFreeInodeCSVRecord(num_block);
                }
                // Set portion of inode array to note that inode block is allocated
                else {
                    inode_array[i][j] = 1;
                }
                // Shift the bits to the right by 1 to read next bit
                inode_bitmap_buf = inode_bitmap_buf >> 1;
                // Increment the block number
                num_block++;
                // Decrement the bit size as a bit was just read
                bit_size--;
            }
            j++;
        }
        // Reset block number for next block group
        num_block = 0;
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
getInodeSummary(void) {
    int i, j, k, block_size = super_block.s_log_block_size;
    
    // Number of inodes per group
    num_inodes = super_block.s_inodes_per_group;
    
    // Create inode table array
    inode_table = (struct ext2_inode **) malloc(sizeof(struct ext2_inode *) * num_groups);
    for (i = 0; i < num_groups; i++) {
        inode_table[i] = (struct ext2_inode *) malloc(sizeof(struct ext2_inode) * num_inodes);
    }
    
    ssize_t nread;
    for (i = 0; i < num_groups; i++) {
        for (j = 0; j < num_inodes; j++) {
            // Check to see if inode is free/allocated
            if (inode_array[i][j] == 0) {
                continue;
            }
            else if (inode_array[i][j] == 1) {
                // For each group, read in a single inode at a time
                nread = pread(image_fd, &inode_buf, INODE_BUFFER_SIZE, block_group[i].g_inode_table + (j * INODE_SIZE));
                if (nread < 0) {
                    fprintf(stderr, "Error reading free inode bitmap info from image file.\n");
                    exit(EXIT_FAILURE);
                }
                
                // Inode ID number (to be analyzed in printInodeSummaryCSVRecord)
                // Stored in inode_array
                
                // File type and mode (to be analyzed in printInodeSummaryCSVRecord)
                memcpy(&inode_table[i][j].i_mode, inode_buf, 2);
                
                // Owner
                memcpy(&inode_table[i][j].i_uid, inode_buf + 2, 2);
                
                // Group
                memcpy(&inode_table[i][j].i_gid, inode_buf + 24, 2);
                
                // Link count
                memcpy(&inode_table[i][j].i_links_count, inode_buf + 26, 2);
                
                // Creation time (to be analyzed in printInodeSummaryCSVRecord)
                memcpy(&inode_table[i][j].i_ctime, inode_buf + 12, 4);
                
                // Modification time (to be analyzed in printInodeSummaryCSVRecord)
                memcpy(&inode_table[i][j].i_mtime, inode_buf + 16, 4);
                
                // Last access time (to be analyzed in printInodeSummaryCSVRecord)
                memcpy(&inode_table[i][j].i_atime, inode_buf + 8, 4);
                
                // File size
                memcpy(&inode_table[i][j].i_size, inode_buf + 4, 4);
                memcpy(&inode_table[i][j].i_dir_acl, inode_buf + 108, 4);
                
                // Number of blocks
                memcpy(&inode_table[i][j].i_blocks, inode_buf + 28, 4);
                
                // Direct blocks
                // Indirect block
                // Doubly indirect block
                // Triply indirect block
                for (k = 0; k < EXT2_N_BLOCKS; k++) {
                    memcpy(&inode_table[i][j].i_block[k], inode_buf + 40 + (k * 4), 4);
                }
            }
        }
    }
}

void
printInodeSummaryCSVRecord(void) {
    // Print to STDOUT inode summary CSV record
    int i, j, k, mode, bitmask;
    //long file_size;
    char file_type;
    time_t create_time, mod_time, access_time;
    struct tm *create_gtime, *mod_gtime, *access_gtime;
    char ctime[TIME_BUFFER], mtime[TIME_BUFFER], atime[TIME_BUFFER];
    for (i = 0; i < num_groups; i++) {
        for (j = 0; j < num_inodes; j++) {
            // Check to see if inode is free/allocated
            // If free, skip
            if (inode_array[i][j] == 0) {
                continue;
            }
            // If allocated, print
            else if (inode_array[i][j] == 1) {
                // Get file type
                bitmask = 0xF000;
                mode = inode_table[i][j].i_mode & bitmask;
                if (mode == EXT2_S_IFREG) {
                    file_type = 'f';
                }
                else if (mode == EXT2_S_IFDIR) {
                    file_type = 'd';
                }
                else if (mode == EXT2_S_IFLNK) {
                    file_type = 's';
                }
                else {
                    file_type = '?';
                }
                
                // Get file permissions mode
                bitmask = 0x0FFF;
                mode = mode & bitmask;
                
                // Get creation time
                create_time = inode_table[i][j].i_ctime;
                create_gtime = gmtime(&create_time);
                if (create_gtime->tm_year > 100 && create_gtime->tm_year < 1000) {
                    create_gtime->tm_year -= 100;
                }
                else if (create_gtime->tm_year > 1000) {
                    create_gtime->tm_year -= 1000;
                }
                create_gtime->tm_mon += 1;
                snprintf(ctime, 18, "%02d/%02d/%02d %02d:%02d:%02d", create_gtime->tm_mon, create_gtime->tm_mday, create_gtime->tm_year, create_gtime->tm_hour, create_gtime->tm_min, create_gtime->tm_sec);
                
                // Get modification time
                mod_time = inode_table[i][j].i_mtime;
                mod_gtime = gmtime(&mod_time);
                if (mod_gtime->tm_year > 100 && mod_gtime->tm_year < 1000) {
                    mod_gtime->tm_year -= 100;
                }
                else if (mod_gtime->tm_year > 1000) {
                    mod_gtime->tm_year -= 1000;
                }
                mod_gtime->tm_mon += 1;
                snprintf(mtime, 18, "%02d/%02d/%02d %02d:%02d:%02d", create_gtime->tm_mon, create_gtime->tm_mday, create_gtime->tm_year, create_gtime->tm_hour, create_gtime->tm_min, create_gtime->tm_sec);
                
                // Get last access time
                access_time = inode_table[i][j].i_atime;
                access_gtime = gmtime(&access_time);
                if (access_gtime->tm_year > 100 && access_gtime->tm_year < 1000) {
                    access_gtime->tm_year -= 100;
                }
                else if (access_gtime->tm_year > 1000) {
                    access_gtime->tm_year -= 1000;
                }
                access_gtime->tm_mon += 1;
                snprintf(atime, 18, "%02d/%02d/%02d %02d:%02d:%02d", create_gtime->tm_mon, create_gtime->tm_mday, create_gtime->tm_year, create_gtime->tm_hour, create_gtime->tm_min, create_gtime->tm_sec);
                
                // File size
                //file_size = (inode_table[i][j].i_dir_acl << 32) | inode_table[i][j].i_size;
                file_size = inode_table[i][j].i_size;

                // Print CSV record
                fprintf(stdout, "%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", "INODE", inode_array[i][j], file_type, mode, inode_table[i][j].i_uid, inode_table[i][j].i_gid, inode_table[i][j].i_links_count, ctime, mtime, atime, file_size, inode_table[i][j].i_blocks);
                for (k = 0; k < EXT2_N_BLOCKS; k++) {
                    fprintf(stdout, ",%d", inode_table[i][j].i_block[k]);
                    if (k == (EXT2_N_BLOCKS - 1)) {
                        fprintf(stdout, "\n");
                    }
                }
            }
        }
    }
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
    getBlockGroup();
    printBlockGroupCSVRecord();
    
    // Get free block information and print it to STDOUT (handled in getFreeBlock())
    getFreeBlock();
    
    // Get free inode information and print it to STDOUT
    getFreeInode();
    printInodeSummaryCSVRecord();
    
    // Get inode summary information and print it to STDOUT
    getInodeSummary();
    printInodeSummaryCSVRecord();
    
    // If success
    exit(EXIT_SUCCESS);
}
