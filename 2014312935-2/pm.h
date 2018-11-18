//----------------------------------------------------------
//
// Project #2 : Page mapping FTL
// 	- Embedded Systems Design, ICE3028 (Spring, 2017)
//
// Apr. 12, 2017.
//
// Dong-Yun Lee (dongyun.lee@csl.skku.edu)
// Jin-Soo Kim (jinsookim@skku.edu)
// Computer Systems Laboratory
// Sungkyunkwan University
// http://csl.skku.edu/ICE3028S17
//
//---------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define SSD_SHIFT	          	32
#define PAGE_SHIFT	           	12
#define PAGES_PER_BLOCK_SHIFT	7
#define OP_RATIO				25
#define N_RUNS                 	10

#define PPN_SHIFT				(SSD_SHIFT - PAGE_SHIFT)
#define BLOCKS_SHIFT			(PPN_SHIFT - PAGES_PER_BLOCK_SHIFT)
#define N_PAGE_SIZE				(1 << PAGE_SHIFT)
#define N_PAGES_PER_BLOCK		(1 << PAGES_PER_BLOCK_SHIFT)
#define N_PPNS				(1 << PPN_SHIFT)
#define N_BLOCKS				(1 << BLOCKS_SHIFT)
#define N_USER_BLOCKS           		(N_BLOCKS * 100 / (100 + OP_RATIO))
#define N_OP_BLOCKS			(N_BLOCKS - N_USER_BLOCKS)
#define N_LPNS				(N_USER_BLOCKS * N_PAGES_PER_BLOCK)
#define LPN_COUNTS				(N_LPNS * N_RUNS)

typedef unsigned int u32;

struct pm_stat {
	int gc;
	long host_write;
	long gc_write;
} s; 

void ftl_open (void);
void ftl_write (long lpn, u32 *data);
void ftl_read (long lpn, u32 *data);
