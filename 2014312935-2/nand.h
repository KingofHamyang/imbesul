//----------------------------------------------------------
//
// Project #1 : NAND Simulator
// 	- Embedded Systems Design, ICE3028 (Spring, 2017)
//
// Mar. 25, 2017.
//
// Dong-Yun Lee (dongyun.lee@csl.skku.edu)
// Jin-Soo Kim (jinsookim@skku.edu)
// Computer Systems Laboratory
// Sungkyunkwan University
// http://csl.skku.edu/ICE3028S17
//
//---------------------------------------------------------


typedef unsigned int 	u32;


// function prototypes
int nand_init (int nblks, int npages);
int nand_read (int blk, int page, u32 *data, u32 *spare);
int nand_write (int blk, int page, u32 data, u32 spare);
int nand_erase (int blk);
int nand_blkdump (int blk);

