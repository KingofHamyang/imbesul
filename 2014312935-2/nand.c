//----------------------------------------------------------
//
// Project #1 : NAND Simulator
// 	- Embedded Systems Design, ICE3028 (Fall, 2018)
//
// Sep. 27, 2018.
//
// Min-Woo Ahn (minwoo.ahn@csl.skku.edu)
// Dong-Hyun Kim (donghyun.kim@csl.skku.edu)
// Dong-Yun Lee (dongyun.lee@csl.skku.edu)
// Jin-Soo Kim (jinsookim@skku.edu)
// Computer Systems Laboratory
// Sungkyunkwan University
// http://csl.skku.edu/ICE3028F18
//
//---------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nand.h"

struct nand_page
{
	u32 data;  // 4B for data
	u32 spare; // 4B for spare
};

struct nand_page **SSD;
int *valid;
int *sequence;
int nb;
int np;

int nand_init(int nblks, int npages)
{

	if (nblks < 0 || npages < 0)
		return -1;
	int i;
	nb = nblks;
	np = npages;
	// initialize the NAND flash memory
	// "blks": the total number of flash blocks
	// "npages": the number of pages per block
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	SSD = (struct nand_page **)malloc(sizeof(struct nand_page *) * nblks);
	for (i = 0; i < nblks; i++)
	{
		SSD[i] = (struct nand_page *)malloc(sizeof(struct nand_page) * npages);
	}
	valid = (int *)malloc(sizeof(int) * nblks);
	sequence = (int *)malloc(sizeof(int) * nblks);

	for (i = 0; i < nblks; i++)
	{
		valid[i] = 1;
	}
	for (i = 0; i < nblks; i++)
	{
		sequence[i] = 0;
	}
	printf("NAND: %d blocks , %d pages per block, %d pages\n", nblks, npages, npages * nblks);

	return 0;
}

int nand_write(int blk, int page, u32 data, u32 spare)
{

	// write "data" and "spare" into the NAND flash memory pointed to by "blk" and "page"
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	if (blk >= nb || blk < 0)
	{
		//printf("write(%d,%d): failed, invalid block number!\n", blk, page);
		return -1;
	}
	if (page >= np || page < 0)
	{
		//	printf("write(%d,%d): failed, invalid page number!\n", blk, page);
		return -1;
	}
	if (sequence[blk] == page)
	{
		sequence[blk]++;
		//	printf("wirte(%d %d): data = %08x, spare = %08x \n", blk, page, data, spare);
		SSD[blk][page].data = data;
		SSD[blk][page].spare = spare;
	}
	else
	{
		if (sequence[blk] < page)
			//	printf("write(%d,%d): failed, the page is not being sequentially written!\n", blk, page);
			//else
			//	printf("write(%d,%d): failed, the page was already written!\n", blk, page);
			return -1;
	}

	return 0;
}

int nand_read(int blk, int page, u32 *data, u32 *spare)
{

	// read "data" and "spare" from the NAND flash memory pointed to by "blk" and "page"
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	if (blk >= nb || blk < 0)
	{
		//	printf("read(%d,%d): failed, invalid block number!\n", blk, page);
		return -1;
	}
	if (page >= np || page < 0)
	{
		//printf("read(%d,%d): failed, invalid page number!\n", blk, page);
		return -1;
	}
	if (sequence[blk] <= page)
	{
		//printf("read(%d,%d): failed, trying to read an empty page\n", blk, page);
		return -1;
	}
	//	printf("read(%d,%d): data = %08x spare = %08x\n", blk, page, SSD[blk][page].data, SSD[blk][page].spare);
	*data = SSD[blk][page].data;
	*spare = SSD[blk][page].spare;

	return 0;
}

int nand_erase(int blk)
{

	// erase the NAND flash memory block "blk"
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message

	if (blk >= nb || blk < 0)
	{
		//		printf("erase(%d): failed, invalid block number!\n", blk);
		return -1;
	}
	else if (sequence[blk] == 0)
	{
		//	printf("erase(%d): failed, trying to erase a free block\n", blk);
		return -1;
	}
	else
	{
		//	printf("erase(%d): block erased\n", blk);
		sequence[blk] = 0;
	}

	return 0;
}

int nand_blkdump(int blk)
{
	int i;
	// dump the contents of the NAND flash memory block "blk" (for debugging purpose)
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	if (blk >= nb || blk < 0)
	{
		//	printf("BLK(%d): failed, invalid block number!\n", blk);
		return -1;
	}

	if (sequence[blk] == 0)
	{
		//	printf("BLK %d : FREE\n", blk);
	}
	else
	{

		//printf("BLK %d : total %d pages written\n", blk, sequence[blk]);
		for (i = 0; i < sequence[blk]; i++)
		{
			//	printf("    Page   %d: data = %08x spare = %08x\n", i, SSD[blk][i].data, SSD[blk][i].spare);
		}
	}

	return 0;
}