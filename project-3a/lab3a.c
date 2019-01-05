#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "ext2_fs.h"


int printDebugging = 1;
unsigned int blockSize;
uint32_t blockNum;
uint32_t inodeNum;
uint16_t inodeSize;
uint32_t blocksPerGroup;
uint32_t inodesPerGroup;
uint32_t firstInode;
unsigned int groupNum;

int **inodes;
uint32_t** gd;

struct ext2_super_block superblockPtr;
struct ext2_group_desc groupDescPtr;
struct ext2_dir_entry direntDescPtr;

int level1call = 0;
uint32_t directBlocks = 0;
uint32_t offsetIndirect;


void superblockSummary(int fileSys)
{

	if(pread(fileSys, &superblockPtr, sizeof(struct ext2_super_block), 1024) < 0)
	{
        fprintf(stderr, "Error reading from file system. %s\n", strerror(errno));
        exit(2);
	}

	blockSize = EXT2_MIN_BLOCK_SIZE << (superblockPtr).s_log_block_size;

    blockNum = superblockPtr.s_blocks_count;
    inodeNum = superblockPtr.s_inodes_count;
    inodeSize = superblockPtr.s_inode_size;
    blocksPerGroup = superblockPtr.s_blocks_per_group;
    inodesPerGroup = superblockPtr.s_inodes_per_group;
    firstInode = superblockPtr.s_first_ino;

    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",	blockNum, (superblockPtr).s_inodes_count, blockSize, (superblockPtr).s_inode_size,	(superblockPtr).s_blocks_per_group, (superblockPtr).s_inodes_per_group, (superblockPtr).s_first_ino);
}

void bfreeSummary(int fileSys, uint32_t bitmap, uint32_t blockOffset)
{
    unsigned long i;
    for(i = 0; i < blockSize; i++)
    {
        uint8_t byteIn;
        if(pread(fileSys, &byteIn, 1, (blockSize*bitmap) + i) < 0)
        {
            fprintf(stderr, "Error reading from file system. %s\n", strerror(errno));
            exit(2);
        }

        uint16_t mask;
        unsigned long itr = 0;
        for(mask = 0x01; mask != 0x100; mask <<= 1)
        {
            if((byteIn & mask) == 0)
            {
                printf("BFREE,%lu\n", blockOffset + i*8 + itr + 1);
            }

            itr++;
        }
    }

    return;
}


void direntSummary(int fileSys, uint32_t parent_block, struct ext2_inode *inodePtr)
{
	uint32_t i;
	for (i = 0; i < EXT2_N_BLOCKS; i++)
	{
		off_t de_head_offset = (blockSize * (inodePtr->i_block[i]));
		off_t de_tail_offset = blockSize * (inodePtr->i_block[i]+1);

		if(de_head_offset == 0 || de_tail_offset == 0)
				break;

		uint32_t logical_offset = 0;
		do
		{
			if (pread(fileSys, &direntDescPtr, sizeof(struct ext2_dir_entry), de_head_offset) < 0)
			{
				fprintf(stderr, "pread() failed in direntSummary: %s\n", strerror(errno));
				exit(1);
			}

			if(direntDescPtr.name_len == 0)
			{
					break;
			}


			printf("DIRENT,%u,%u,%u,%u,%u,'%s'\n",
				parent_block, logical_offset, direntDescPtr.inode, direntDescPtr.rec_len,
				direntDescPtr.name_len, direntDescPtr.name);

			de_head_offset += direntDescPtr.rec_len;
			logical_offset += direntDescPtr.rec_len;

		} while (direntDescPtr.rec_len != 0 && de_head_offset < de_tail_offset);
	}
}

void indirectSummary(int fileSys, int inodeID, uint32_t blockID, int level, uint32_t offset)
{
	uint32_t entryNum = blockSize/sizeof(uint32_t);
	uint32_t *entries;

	entries = malloc(sizeof(uint32_t)*entryNum);

	unsigned int k;
	for(k = 0; k < entryNum; k++)
	{
		entries[k] = 0;
	}

	if(pread(fileSys, entries, blockSize, 1024+(blockID-1)*blockSize) < 0)
	{
		fprintf(stderr, "Error reading from file system. %s\n", strerror(errno));
		exit(2);
	}

	unsigned int i;
	for(i = 0; i < entryNum; i++)
	{
		if(entries[i] != 0)
		{
			printf("INDIRECT,%u,%u,%u,%u,%u\n", inodeID, level, offset, blockID, entries[i]);

			switch (level) {
				case 1:
					if(level1call)
						directBlocks++;

					offset++;
					break;
				case 2:
					offset += 2;
					indirectSummary(fileSys, inodeID, entries[i], 1, offset);
					break;
				case 3:
					indirectSummary(fileSys, inodeID, entries[i], 2, offset);
					break;
				default:
					fprintf(stderr, "Error, given indirect level %d\n", level);
					exit(2);
			}
		}
	}

	free(entries);

    return;
}

void inodeSummary(int fileSys, int inodeID, int groupID)
{
	struct ext2_inode currInode;

	if(pread(fileSys, &currInode, sizeof(struct ext2_inode), 1024 + ((*(gd[groupID])-1)*blockSize) + (inodeID-1)*sizeof(struct ext2_inode)) < 0)
	{
		fprintf(stderr, "Error reading from file system. %s\n", strerror(errno));
		exit(2);
	}

	uint16_t inodeMode = currInode.i_mode & 0x01FF;

	char createT[80];
	time_t timestamp = currInode.i_ctime;
	struct tm cTime = *gmtime(&timestamp);
	strftime(createT, 80, "%m/%d/%y %H:%M:%S", &cTime);

	char modifyT[80];
	timestamp = currInode.i_mtime;
	struct tm mTime = *gmtime(&timestamp);
	strftime(modifyT, 80, "%m/%d/%y %H:%M:%S", &mTime);

	char accessT[80];
	timestamp = currInode.i_atime;
	struct tm aTime = *gmtime(&timestamp);
	strftime(accessT, 80, "%m/%d/%y %H:%M:%S", &aTime);

	char fType;

	if (S_ISDIR(currInode.i_mode))
		fType = 'd';
	else if (S_ISREG(currInode.i_mode))
		fType = 'f';
	else if (S_ISLNK(currInode.i_mode))
		fType = 's';
	else
		fType = '?';

	printf("INODE,%d,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u", inodeID, fType, inodeMode,	currInode.i_uid, currInode.i_gid, currInode.i_links_count, createT, modifyT, accessT, currInode.i_size, currInode.i_blocks);

	int k;
	for (k = 0; k < EXT2_N_BLOCKS; k++) {
		printf(",%u", currInode.i_block[k]);
	}
	printf("\n");

	if(fType == 'd')
	{
		direntSummary(fileSys, inodeID, &currInode);
	}

	if(currInode.i_block[EXT2_IND_BLOCK] != 0)
	{
		level1call = 1;
		directBlocks = 0;
		indirectSummary(fileSys, inodeID, currInode.i_block[EXT2_IND_BLOCK], 1, offsetIndirect);
		offsetIndirect += directBlocks;
	}

	if(currInode.i_block[EXT2_DIND_BLOCK] != 0)
	{
		level1call = 0;
		indirectSummary(fileSys, inodeID, currInode.i_block[EXT2_DIND_BLOCK], 2, 268);
	}

	if(currInode.i_block[EXT2_TIND_BLOCK] != 0)
	{
		level1call = 0;
		indirectSummary(fileSys, inodeID, currInode.i_block[EXT2_TIND_BLOCK], 3, 65804);
	}

    return;
}

bool validInode(unsigned int inodeNumber, int index)
{
	bool isValid = true;
	bool isPresent = false;

	unsigned int i;
	for(i = 0; i < blockSize; i++)
	{
		uint8_t byteIn = *(inodes[index+i]);

		int itr;
		uint8_t j = 0x01;
		for (itr = 0; itr < 8; itr++)
		{
			unsigned long tempID = index*inodesPerGroup + i*8 + itr + 1;
			if(tempID == inodeNumber)
			{
				isPresent = true;
				if((byteIn & j) == 0)
					isValid = false;

				break;
			}

			j <<= 1;
			if(isPresent)
				break;
		}
	}

	if(!isPresent)
	{
		fprintf(stderr, "Error, bitmap is corrupted. %d", inodeNumber);
		exit(2);
	}


	return isValid;
}

void inodeSetup(int fileSys)
{
	unsigned int i;
	offsetIndirect = 12;
	for(i = 0; i < groupNum; i++)
	{
		if(validInode(2, i))
			inodeSummary(fileSys, 2, i);

		unsigned int ii;
		for(ii = firstInode; ii < inodeNum; ii++)
		{

			if(!validInode(ii, i))
			{
				continue;
			}

			inodeSummary(fileSys, ii, i);
		}
	}
}


void ifreeSummary(int fileSys, uint32_t inode_bitmap, uint32_t block_offset, int groupID)
{
	uint32_t offset = inode_bitmap * blockSize;
	uint8_t buf;



	uint32_t i;
	for (i = 0; i < blockSize; i++)
	{
		if (pread(fileSys, &buf, sizeof(uint8_t), offset + i) < 0)
		{
			fprintf(stderr, "pread() failed in ifreeSummary: %s\n", strerror(errno));
		}

		uint16_t j;
		uint32_t itr = 0;

		*(inodes[groupID + i]) = buf;

		for (j = 0x1; j != 0x100; j <<= 1)
		{
			if ((buf & j) == 0)
			{
				printf("IFREE,%u\n", block_offset + i*8 + itr + 1);
			}

			itr++;
		}
	}
}

void groupSummary(int fileSys)
{
    groupNum = blockNum / blocksPerGroup;

    groupNum += (blockNum % blocksPerGroup != 0) ? 1 : 0;

    uint32_t descblock = 0;
    if(blockSize == 1024)
        descblock = 2;
    else
        descblock = 1;

    uint32_t inodesLeft = inodeNum;


	inodes = malloc(sizeof(uint8_t*) * groupNum * blockSize);
	unsigned int j;
	for(j = 0; j < groupNum*blockSize; j++)
	{
		inodes[j] = malloc(sizeof(uint8_t));
	}

	gd = malloc(sizeof(uint32_t*)*groupNum);
	unsigned int k;
	for (k = 0; k < groupNum; k++) {
		gd[k] = malloc(sizeof(uint32_t));
	}


    uint32_t i;
    for(i = 0; i < groupNum; i++)
    {
        // 32 is the size of each group descriptor
        if(pread(fileSys, &groupDescPtr, sizeof(struct ext2_group_desc), blockSize*descblock + 32*i) < 0)
        {
            fprintf(stderr, "Error reading from file system.\n");
            exit(2);
        }

        printf("GROUP,%d,", i);

        if(i != groupNum - 1)
            printf("%u,", blocksPerGroup);
        else
            printf("%u,", blockNum - blocksPerGroup*i);

        if(inodesLeft < inodesPerGroup)
            printf("%u,", inodesLeft);
        else
            printf("%u,", inodesPerGroup);

        printf("%u,%u,%u,%u,%u\n", groupDescPtr.bg_free_blocks_count, groupDescPtr.bg_free_inodes_count, groupDescPtr.bg_block_bitmap, groupDescPtr.bg_inode_bitmap, groupDescPtr.bg_inode_table);

        bfreeSummary(fileSys, groupDescPtr.bg_block_bitmap, blocksPerGroup*i);
        ifreeSummary(fileSys, groupDescPtr.bg_inode_bitmap, i*blocksPerGroup, i);

		*(gd[i]) = groupDescPtr.bg_inode_table;
    }

    return;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Error: wrong number of arguments\n");
		exit(1);
	}

	int fileSys = open(argv[1], O_RDONLY);
	if(fileSys < 0)
	{
		fprintf(stderr, "Error opening img file. %s\n", strerror(errno));
		exit(1);
	}

	superblockSummary(fileSys);
    groupSummary(fileSys);
	inodeSetup(fileSys);

	free(inodes);
	free(gd);

	exit(0);
}
