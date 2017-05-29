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
// Block group descriptor table size
#define DESCRIPTOR_TABLE_SIZE 32
// Size of inode
#define INODE_SIZE 128

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

// Directory format specifiers
#define EXT2_BTREE_FL 4096
#define EXT2_INDEX_FL 4096

// Time field length
#define TIME_BUFFER 18

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Globals ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/* Programmable Variables */
// File descriptor for the image file provided
int image_fd;
// Buffer to acquire information for the super block
char super_block_buf[SUPER_BLOCK_SIZE];
// Buffer to acquire information for the block group descriptor table
char descriptor_table_buf[DESCRIPTOR_TABLE_SIZE];
// Number of block groups
int num_groups;
// Number of inodes per group
int num_inodes;

/* Flags */
// When checking to see if inode is a directory
int directory_flag = 0;

/* Structs */
// Super block struct to store the super block information
struct ext2_super_block super_block;
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
// Array to keep track of which inodes are allocated/free for each group
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
void getInodeAndDirectory(void);
// Print inode summary csv record
void printInodeAndDirectoryCSVRecord(void);

// Acquire indirect block references

// Print indirect block references csv record
int printIndirectCSVRecord(int fileOffset, int iniBlockNum, int ParentLevel, int i, int j);
int checkDoubleIndirectBlock (int fileOffset, int iniBlockNum, int ParentLevel, int i, int j);
int checkDoubleIndirectBlock (int fileOffset, int iniBlockNum, int ParentLevel, int i, int j);
void getIndirectBlocks(void);

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
print_usage(void) {
    printf("Usage: ./lab3a [Image File]\n");
}

//TODO: fix exit codes for parser
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
    memset(super_block_buf, 0, SUPER_BLOCK_SIZE);
    
    ssize_t nread = pread(image_fd, super_block_buf, SUPER_BLOCK_SIZE, SUPER_BLOCK_OFFSET);
    if (nread < 0) {
        fprintf(stderr, "Error reading super block info from image file.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Debugging */
    //memcpy(&block->s_rev_level, super_block_buf + 76, 4);

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
    memcpy(&block->s_first_ino, super_block_buf + 84, 4);
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

    /* Debugging */
    //printf("Blocks: %d\n", super_block.s_blocks_count);
    //printf("Blocks per group: %d\n", super_block.s_blocks_per_group);
    //printf("Number of groups: %d\n", num_groups);
    //printf("Revision number: %d\n", super_block.s_rev_level);

    // If there are some leftover blocks for last block group
    int leftover_blocks = super_block.s_blocks_count % super_block.s_blocks_per_group;

    // Create block group array
    block_group = (struct p3_block_group *) malloc(sizeof(struct p3_block_group) * num_groups);
    
    if (block_group == NULL) {
        fprintf(stderr, "Error: failed malloc for block_group.\n");
        exit(EXIT_FAILURE);
    }

    ssize_t nread;
    int i, last_group = num_groups - 1;
    for (i = 0; i < num_groups; i++) {
        // Initialize the buffer to read in for descriptor table
        memset(descriptor_table_buf, 0, DESCRIPTOR_TABLE_SIZE);
        
        nread = pread(image_fd, descriptor_table_buf, DESCRIPTOR_TABLE_SIZE, SUPER_BLOCK_SIZE + SUPER_BLOCK_OFFSET + (i * DESCRIPTOR_TABLE_SIZE));
        if (nread < 0) {
            fprintf(stderr, "Error reading block group descriptor table info from image file.\n");
            exit(EXIT_FAILURE);
        }
        
        // Group ID number
        block_group[i].group_id = i;
        
        // Total number of blocks (if there are leftover blocks, last group has the leftover)
        if (leftover_blocks == 0) {
        	block_group[i].g_blocks_count = super_block.s_blocks_per_group;
        }
        else if ((leftover_blocks != 0) && (i == last_group)) {
        	block_group[i].g_blocks_count = leftover_blocks;
        }

        
        // Total number of inodes
        block_group[i].g_inodes_count = super_block.s_inodes_per_group;
        
        // Total number of free blocks
        memcpy(&block_group[i].g_free_blocks_count, descriptor_table_buf + 12, 2);

        // Total number of free inodes
        memcpy(&block_group[i].g_free_inodes_count, descriptor_table_buf + 14, 2);

        /* Debugging */
        //printf("Number of free blocks: %d\n", block_group[i].g_free_blocks_count);
        //printf("Number of free inodes: %d\n", block_group[i].g_free_inodes_count);

        // Block number of free block bitmap
        memcpy(&block_group[i].g_block_bitmap, descriptor_table_buf, 4);
        
        // Block number of free inode bitmap
        memcpy(&block_group[i].g_inode_bitmap, descriptor_table_buf + 4, 4);
        
        // Block number of first block of inodes
        memcpy(&block_group[i].g_inode_table, descriptor_table_buf + 8, 4);
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
    int i, j, bit_size = 8, block_size = super_block.s_log_block_size, num_block = 1, bitmask = 1;

    /* Debugging */
    //printf("Block size: %d\n", block_size);
    // Block size: 1024

    for (i = 0; i < num_groups; i++) {
        j = 0;
        // Loop through block bitmap (1024 bytes)
        while (j != block_size) {
            block_bitmap_buf = 0;
            // Read in a byte at a time (8 bits: 8 corresponding data blocks)
            // Each block bitmap is located at block number: block_group[i].g_block_bitmap
            // This is just the block number, so multiply by block size (1024) to get to block bitmap
            // Now: since we read in a byte a time, increment the offset by 1 from current location each time (i.e +j)
            nread = pread(image_fd, &block_bitmap_buf, 1, (block_group[i].g_block_bitmap * block_size) + j);
            
            /* Debugging */
            //printf("Block buf contains: %d\n", block_bitmap_buf);
            //printf("Block bitmap location: %d\n", (block_group[i].g_block_bitmap * block_size) + j);

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
                if ((block_bitmap_buf & bitmask) == 0) {
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
    int i, j, k, bit_size = 8, block_size = super_block.s_log_block_size, num_block = 1, bitmask = 1;
    
    // Initialize the inode free/allocated array
    num_inodes = super_block.s_inodes_per_group;
    inode_array = (int **) malloc(sizeof(int *) * num_groups);

    if (inode_array == NULL) {
        fprintf(stderr, "Error: failed malloc for inode_array.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_groups; i++) {
        inode_array[i] = (int *) malloc(sizeof(int) * num_inodes);
        //fprintf(stderr, "%d\n", num_inodes);
        if (inode_array[i] == NULL) {
            fprintf(stderr, "Error: failed malloc for inode_array[%d].\n", i);
            exit(EXIT_FAILURE);
        }
    }

    /* Debugging */
    //printf("Number of inodes: %d\n", num_inodes);

    // Variable to hold the bitmap block (reading 1 byte at a time)
    __u8 inode_bitmap_buf;
    
    ssize_t nread;
    for (i = 0; i < num_groups; i++) {
        k = 0;
        for (j = 0; j < ceil(num_inodes/8); j++) {
            inode_bitmap_buf = 0;
            nread = pread(image_fd, &inode_bitmap_buf, 1, (block_group[i].g_inode_bitmap * block_size) + j);
            if (nread < 0) {
                fprintf(stderr, "Error reading free inode bitmap info from image file.\n");
                exit(EXIT_FAILURE);
            }

            /* Debugging */
            //printf("Inode buf contains: %d\n", inode_bitmap_buf);
            //printf("Inode bitmap location: %d\n", (block_group[i].g_inode_bitmap * block_size) + j);

            // No more bytes to read from bitmap
            if (nread == 0) {
                bit_size = 0;
            }
            else {
                bit_size = 8;
            }
            //fprintf(stdout, "%d\n", inode_bitmap_buf);
            while (bit_size != 0) {
                // If the bit being checked is a 0, then the corresponding block is free
                if ((inode_bitmap_buf & bitmask) == 0) {
                    // Set portion of inode array to note that inode block is free
                    inode_array[i][k] = 0;
                    //fprintf(stdout, "k = %d, num_block = %d, inode_array[%d][%d] = %d\n", k, num_block, i, j, inode_array[i][j]);
                    printFreeInodeCSVRecord(num_block);
                }
                // Set portion of inode array to note that inode block is allocated
                else {
                    inode_array[i][k] = 1;
                    //fprintf(stdout, "k = %d, num_block = %d, inode_array[%d][%d] = %d\n", k, num_block, i, j, inode_array[i][j]);
                }
                // Shift the bits to the right by 1 to read next bit
                inode_bitmap_buf = inode_bitmap_buf >> 1;
                // Increment the block number
                num_block++;
                // Decrement the bit size as a bit was just read
                bit_size--;
                // Increment the position within the inode_array                
                k++;
            }
        }
        // Reset block number for next block group
        num_block = 0;
    }

    /* Debugging */
    //for (i = 0; i < num_groups; i++) {
    //	for (k = 0; k < num_inodes; k++) {
    //		printf("Inode Group: %d, Inode Number: %d => %d\n", i, k + 1, inode_array[i][k]);
    //
    //	}
    //}
}

void
printFreeInodeCSVRecord(int num_block) {
    // Print to STDOUT free inodes CSV record
    fprintf(stdout, "%s,%d\n", "IFREE", num_block);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////// Inode And Directory Summary /////////////////////
////////////////////////////////////////////////////////////////////////////
void
getInodeAndDirectory(void) {
	int i, j, k, block_size = super_block.s_log_block_size;
    
    // Number of inodes per group
    num_inodes = super_block.s_inodes_per_group;
    
    // Create inode table array
    inode_table = (struct ext2_inode **) malloc(sizeof(struct ext2_inode *) * num_groups);
    
    if (inode_table == NULL) {
        fprintf(stderr, "Error: failed malloc for inode_table.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_groups; i++) {
        inode_table[i] = (struct ext2_inode *) malloc(sizeof(struct ext2_inode) * num_inodes);
        if (inode_table[i] == NULL) {
            fprintf(stderr, "Error: failed malloc for inode_table[%d].\n", i);
            exit(EXIT_FAILURE);
        }
    }

    ssize_t nread;
    for (i = 0; i < num_groups; i++) {
        for (j = 0; j < num_inodes; j++) {
            // Check to see if inode is free/allocated
            if (inode_array[i][j] == 0) {
                continue;
            }
            else if (inode_array[i][j] == 1) {
            	/* Debugging */
            	//printf("Inode Group: %d, Inode Number: %d\n", i, j + 1);


                // For each group, read in a single inode at a time
                nread = pread(image_fd, &inode_table[i][j], INODE_SIZE, (block_group[i].g_inode_table * block_size) + (j * INODE_SIZE));
                if (nread < 0) { 
                    fprintf(stderr, "Error reading free inode bitmap info from image file.\n");
                    exit(EXIT_FAILURE);
                }                

                /* Debugging */
                /*
                printf("Inode file type and mode: %d\n", inode_table[i][j].i_mode);
                printf("Inode Owner: %d\n", inode_table[i][j].i_uid);
                printf("Inode Group: %d\n", inode_table[i][j].i_gid);
                printf("Inode Link Count: %d\n", inode_table[i][j].i_links_count);
                printf("Inode Creation Time: %d\n", inode_table[i][j].i_ctime);
                printf("Inode Modification Time: %d\n", inode_table[i][j].i_mtime);
                printf("Inode Access Time: %d\n", inode_table[i][j].i_atime);
                printf("Inode Size: %d\n", inode_table[i][j].i_size);
                printf("Inode Blocks: %d\n", inode_table[i][j].i_blocks);
				*/
            }
        }
    }
}

void
printInodeAndDirectoryCSVRecord(void) {
    // Print to STDOUT inode summary CSV record
    int i, j, k, l, file_mode, mode, flags, bitmask, block_size = super_block.s_log_block_size;
    ssize_t nread;
    long int file_size;
    char file_type;
    time_t create_time, mod_time, access_time;
    struct tm *create_gtime, *mod_gtime, *access_gtime;
    char ctime[TIME_BUFFER], mtime[TIME_BUFFER], atime[TIME_BUFFER];
    for (i = 0; i < num_groups; i++) {
        for (j = 0; j < num_inodes; j++) {
  			// Reset directory_flag
  			directory_flag = 0;

            // Check to see if inode is free/allocated
            // If free, skip
            if (inode_array[i][j] == 0) {
                continue;
            } 
            // If allocated, print
            else if (inode_array[i][j] == 1) {
            	// Check to see if nonzero mode
            	if ((inode_table[i][j].i_mode == 0) || (inode_table[i][j].i_links_count == 0))
            		continue;

            	/* Debugging */
            	// Get directory format
            	/*bitmask = 0x00F0;
            	flags = inode_table[i][j].i_flags & bitmask;
            	if (flags == EXT2_INDEX_FL) {
            		printf("Directory format is indexed.\n");
            	}
            	else
            		printf("Directory format is linked list.\n");
				*/

                // Get file type
                bitmask = 0xF000;
                mode = inode_table[i][j].i_mode;
                file_mode = mode & bitmask;
                if (file_mode == EXT2_S_IFREG) {
                    file_type = 'f';
                }
                else if (file_mode == EXT2_S_IFDIR) {
                    file_type = 'd';
                    directory_flag = 1;
                }
                else if (file_mode == EXT2_S_IFLNK) {
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
                file_size = inode_table[i][j].i_dir_acl;
                file_size = (file_size << 32) | inode_table[i][j].i_size;

                // Print CSV record
                fprintf(stdout, "%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%li,%d", "INODE", j + 1, file_type, mode, inode_table[i][j].i_uid, inode_table[i][j].i_gid, inode_table[i][j].i_links_count, ctime, mtime, atime, file_size, inode_table[i][j].i_blocks);
                for (k = 0; k < EXT2_N_BLOCKS; k++) {
                    fprintf(stdout, ",%d", inode_table[i][j].i_block[k]);
                    if (k == (EXT2_N_BLOCKS - 1)) {
                        fprintf(stdout, "\n");
                    }
                }

                // If inode is a directory, process it
                if (directory_flag == 1) {
                	struct ext2_dir_entry directory;
                    // Analyze direct blocks 
                	for (k = 0; k < EXT2_N_BLOCKS - 3; k++) {
                		int offset = 0; 
                		while (offset < block_size) {
                			nread = pread(image_fd, &directory, sizeof(struct ext2_dir_entry), (inode_table[i][j].i_block[k] * block_size) + offset);
               				if (nread < 0) { 
                    			fprintf(stderr, "Error reading block directory info from image file.\n");
                    			exit(EXIT_FAILURE);
               				}   

                            // Check to see that the data block is valid
               				if (directory.inode == 0)
               					break;

                            // Add 1 for null-terminating character
                            // Add 2 for single quotes
                            char file_name[EXT2_NAME_LEN + 1 + 2];
                            file_name[0] = '\'';
                            memcpy(file_name + 1, directory.name, directory.name_len);
                            file_name[directory.name_len+1] = '\'';
                            file_name[directory.name_len+2] = '\0';

               				fprintf(stdout, "%s,%d,%d,%d,%d,%d,%s\n", "DIRENT", j + 1, offset, directory.inode, directory.rec_len, directory.name_len, file_name);

                            // Increment the offset to the next directory entry
                			offset += directory.rec_len;
                		}
                	}

                    // Analyze singly indirect block
                    for (k = EXT2_N_BLOCKS - 3; k < EXT2_N_BLOCKS - 2; k++) {
                        nread = pread(image_fd, &directory, sizeof(struct ext2_dir_entry), (inode_table[i][j].i_block[k] * block_size));
                        if (directory.inode == 0)
                            break;

                        __u32 num_block;
                        // Address of the indirect block
                        int indirect_block = inode_table[i][j].i_block[k] * block_size;
                        int increment = 0;
                        int address = 0;

                        /* Debugging */
                        // Address of the indirect block
                        // Indirect block: Block 1014
                        //__u32 test;
                        // First block: 1015, Second block: 1016, Third Block: 145, Fourth Block: 146
                        //pread(image_fd, &test, sizeof(__u32), indirect_block + (sizeof(__u32) * 20));
                        //printf("Testing: %d\n", test);

                        pread(image_fd, &num_block, sizeof(__u32), indirect_block);
                        while (num_block != 0) {
                            // Read in the block number
                            address = num_block * block_size;
                            int offset = 0;
                            while (offset < block_size) {
                                pread(image_fd, &directory, sizeof(struct ext2_dir_entry), address + offset);

                                // Check to see that the data block is valid
                                if (directory.inode == 0)
                                    break;

                                // Add 1 for null-terminating character
                                // Add 2 for single quotes
                                char file_name[EXT2_NAME_LEN + 1 + 2];
                                file_name[0] = '\'';
                                memcpy(file_name + 1, directory.name, directory.name_len);
                                file_name[directory.name_len+1] = '\'';
                                file_name[directory.name_len+2] = '\0';

                                fprintf(stdout, "%s,%d,%d,%d,%d,%d,%s\n", "DIRENT", j + 1, offset, directory.inode, directory.rec_len, directory.name_len, file_name);

                                // Increment the offset to the next directory entry
                                offset += directory.rec_len;
                            }
                            increment += sizeof(__u32);
                            pread(image_fd, &num_block, sizeof(__u32), indirect_block + increment);
                        }
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////// Indirect Block References /////////////////////
////////////////////////////////////////////////////////////////////////////

int
printIndirectCSVRecord(int fileOffset, int iniBlockNum, int ParentLevel, int i, int j) {
    // Print to STDOUT indirect block references CSV record

    // get block size for offset
    int k, block_size = 1024 << super_block.s_log_block_size;
    // each block takes up 4 bits of the block size
    int totalBlockNum = block_size/4;    // should be 256

    __u32* singleIndirBlocks = (__u32*)malloc(block_size);

    if (singleIndirBlocks == NULL) {
        fprintf(stderr, "Error: failed malloc for singleIndirBlocks.\n");
    }

    if (pread(image_fd, singleIndirBlocks, block_size, iniBlockNum*block_size) < 0) {
        fprintf(stderr, "Error reading single indirect blocks from image file.\n");
        exit(EXIT_FAILURE);
    }

    for (k = 0; k < totalBlockNum; k++) {
        fileOffset++;
        if (singleIndirBlocks[k] != 0) {
            fprintf(stdout,"%s,%d,%d,%d,%d,%d\n", "INDIRECT", j+1, ParentLevel, fileOffset, iniBlockNum, singleIndirBlocks[k]);
        }
    }
    return fileOffset;
}

int checkDoubleIndirectBlock (int fileOffset, int iniBlockNum, int ParentLevel, int i, int j) {
    // get block size for offset
    int k, block_size = 1024 << super_block.s_log_block_size;
    // each block takes up 4 bits of the block size
    int totalBlockNum = block_size/4;    // should be 256

    __u32* doubleIndirBlocks = (__u32*)malloc(block_size);

    if (doubleIndirBlocks == NULL) {
        fprintf(stderr, "Error: failed malloc for doubleIndirBlocks.\n");
    }        

    if (pread(image_fd, doubleIndirBlocks, block_size, iniBlockNum*block_size) < 0) {
        fprintf(stderr, "Error reading double indirect blocks from image file.\n");
        exit(EXIT_FAILURE);
    }

    for (k = 0; k < totalBlockNum; k++) {
        if (doubleIndirBlocks[k] != 0) {
            fileOffset = printIndirectCSVRecord(fileOffset, doubleIndirBlocks[k], 2, i, j);
        }
    }
}

int checkTripleIndirectBlock (int fileOffset, int iniBlockNum, int ParentLevel, int i, int j) {
    // get block size for offset
    int k, block_size = 1024 << super_block.s_log_block_size;
    // each block takes up 4 bits of the block size
    int totalBlockNum = block_size/4;    // should be 256

    __u32* tripleIndirBlocks = (__u32*)malloc(block_size);

    if (tripleIndirBlocks == NULL) {
        fprintf(stderr, "Error: failed malloc for doubleIndirBlocks.\n");
    }        

    if (pread(image_fd, tripleIndirBlocks, block_size, iniBlockNum*block_size) < 0) {
        fprintf(stderr, "Error reading double indirect blocks from image file.\n");
        exit(EXIT_FAILURE);
    }

    for (k = 0; k < totalBlockNum; k++) {
        if (tripleIndirBlocks[k] != 0) {
            fileOffset = checkDoubleIndirectBlock(fileOffset, tripleIndirBlocks[k], 3, i, j);
        }
    }
}

void
getIndirectBlocks(void) {
    //check indirect blocks 
    int i, j, k, fileOffset = 11, block_size = super_block.s_log_block_size;
    for (i = 0; i < num_groups; i++) {
        for (j = 0; j < num_inodes; j++) {
            // Check to see if inode is free/allocated
            if (inode_array[i][j] == 0) {
                continue;
            }
            // If allocated, check indirect blocks
            else if (inode_array[i][j] == 1) {
                if (inode_table[i][j].i_block[12] != 0) {
                    printIndirectCSVRecord(11, inode_table[i][j].i_block[12], 1, i, j);
                }

                if (inode_table[i][j].i_block[13] != 0) {
                    checkDoubleIndirectBlock(11+block_size/4+1, inode_table[i][j].i_block[13], 2, i, j);
                }

                
                if (inode_table[i][j].i_block[14] != 0) {
                    checkTripleIndirectBlock(11+(block_size/4)+(block_size/4)*(block_size/4), inode_table[i][j].i_block[14], 3, i, j);
                }
            }
        }
    }
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
    
    // Get inode and directory summary information and print it to STDOUT
    getInodeAndDirectory();
    printInodeAndDirectoryCSVRecord();

    // Get Indirect Block summary
    getIndirectBlocks();

    // If success
    exit(EXIT_SUCCESS);
}
