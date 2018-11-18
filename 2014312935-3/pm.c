//----------------------------------------------------------
//
// Project #3 : Other Page mapping FTLs
// 	- Embedded Systems Design, ICE3028 (Fall, 2018)
//
// Apr. 20, 2017.
//
// Dong-Yun Lee (dongyun.lee@csl.skku.edu)
// Jin-Soo Kim (jinsookim@skku.edu)
// Computer Systems Laboratory
// Sungkyunkwan University
// http://csl.skku.edu/ICE3028F18
//
//---------------------------------------------------------

#include "pm.h"
#include "nand.h"

#ifdef COST_BENEFIT
static long now(void) {
	return s.host_write + s.gc_write;
}
#endif


int *L2Ptable;
int *written_pages;
int *PPN_invaild;
int *invalid_pages_per_block;
int full_page_number;
int spare_block_N;
int spare_block[3];
int *block_age;
int *stream_id;
int id_cnt[3] = {0,0,0};


static void garbage_collection(void);


void ftl_open(void)
{
	int a = nand_init(N_BLOCKS, N_PAGES_PER_BLOCK);
	if(a !=0){
		return;
	}
	L2Ptable = (int *)malloc(sizeof(int) * N_USER_BLOCKS * N_PAGES_PER_BLOCK);
	written_pages = (int *)malloc(sizeof(int) * N_BLOCKS);
	PPN_invaild = (int *)malloc(sizeof(int) * N_BLOCKS * N_PAGES_PER_BLOCK);
	invalid_pages_per_block = (int *)malloc(sizeof(int)*N_BLOCKS);
	block_age = (int *)malloc(sizeof(int)*N_BLOCKS);
	stream_id = (int *)malloc(sizeof(int)*N_BLOCKS);

	for (int i = 0; i < N_USER_BLOCKS * N_PAGES_PER_BLOCK; i++)
	{
		L2Ptable[i] = -1;
	}
	for (int i = 0; i <N_BLOCKS; i++)
	{
		written_pages[i] = 0;
		block_age[i] = 0;
		stream_id[i]= -1;
		id_cnt[i%3]++;
		
	}
	
	for(int i = 0 ; i<N_BLOCKS * N_PAGES_PER_BLOCK; i++){
		PPN_invaild[i] = 0;
	}

	spare_block_N = N_BLOCKS - 1;
	spare_block[0] = 0;
	spare_block[1] = 1;
	spare_block[2] = 2;
	//printf("finishi!");
	return;
	
}

void ftl_read(long lpn, u32 *read_buffer)
{
	u32 data;
	u32 spare;
	int blk = L2Ptable[lpn] / N_PAGES_PER_BLOCK;
	int page = L2Ptable[lpn] % N_PAGES_PER_BLOCK;
	//	printf("%d %d\n", blk, page);
	nand_read(blk, page, &data, &spare);

	*read_buffer = data;
	return;
}
#ifdef MULTI_STREAM
void ftl_write (int streamid, long lpn, u32 *write_buffer){
	int full_cnt = 0;

	for(int i = 0 ; i < N_BLOCKS ; i++){
		if(written_pages[i] == N_PAGES_PER_BLOCK){
			full_cnt++;
		}
	}
	if(full_cnt == N_BLOCKS -1){
		garbage_collection();
	
	}
	
	unsigned int temp = lpn;

	if (L2Ptable[lpn] == -1)
	{
		int blk = 0;
		int page = 0;

		for (int i = 0; i < N_BLOCKS; i++)
		{
			if (written_pages[i] != N_PAGES_PER_BLOCK && i != spare_block_N && (stream_id[i] == -1 || stream_id[i] == streamid))
			{
				blk = i;
				page = written_pages[i];
				written_pages[i]++;
				if(stream_id[i] == -1)stream_id[i] = streamid;
				if (written_pages[i] == N_PAGES_PER_BLOCK)
				{
					full_page_number++;
				}
				break;
			}
		}
		L2Ptable[lpn] = blk * N_PAGES_PER_BLOCK + page;
		nand_write(blk, page, *write_buffer, temp);
	}
	else
	{
		int blk = 0;
		int page = 0;
		int old_page_cnt = L2Ptable[lpn];
		PPN_invaild[old_page_cnt] = 1;
		invalid_pages_per_block[old_page_cnt/N_PAGES_PER_BLOCK]++;

		for (int i = 0; i < N_BLOCKS; i++)
		{
			if (written_pages[i] != N_PAGES_PER_BLOCK && i != spare_block_N && (stream_id[i] == -1 || stream_id[i] == streamid))
			{
				blk = i;
				page = written_pages[i];
				if(stream_id[i] == -1)stream_id[i] = streamid;
				written_pages[i]++;
				if (written_pages[i] == N_PAGES_PER_BLOCK)
				{
					full_page_number++;
				}
				break;
			}
		}
		L2Ptable[lpn] = blk * N_PAGES_PER_BLOCK + page;
		nand_write(blk, page, *write_buffer, temp);
	}
	return;
}

#else
void ftl_write(long lpn, u32 *write_buffer)
{
	int full_cnt = 0;

	for(int i = 0 ; i < N_BLOCKS ; i ++){
		if(written_pages[i] == N_PAGES_PER_BLOCK){
			full_cnt++;
		}

	}
	if(full_cnt == N_BLOCKS -1){
		garbage_collection();
		//printf("blk _gc %d", blk_gc);
	}
	unsigned int temp = lpn;

	if (L2Ptable[lpn] == -1)
	{
		int blk = 0;
		int page = 0;

		for (int i = 0; i < N_BLOCKS; i++)
		{
			if (written_pages[i] != N_PAGES_PER_BLOCK && i != spare_block_N)
			{
				blk = i;
				page = written_pages[i];
				written_pages[i]++;
				if (written_pages[i] == N_PAGES_PER_BLOCK)
				{
					full_page_number++;
				}
				break;
			}
		}
		L2Ptable[lpn] = blk * N_PAGES_PER_BLOCK + page;
		nand_write(blk, page, *write_buffer, temp);
	}
	else
	{
		int blk = 0;
		int page = 0;
		int old_page_cnt = L2Ptable[lpn];
		PPN_invaild[old_page_cnt] = 1;
		invalid_pages_per_block[old_page_cnt/N_PAGES_PER_BLOCK]++;

		for (int i = 0; i < N_BLOCKS; i++)
		{
			if (written_pages[i] != N_PAGES_PER_BLOCK && i != spare_block_N)
			{
				blk = i;
				page = written_pages[i];

				written_pages[i]++;
				if (written_pages[i] == N_PAGES_PER_BLOCK)
				{
					full_page_number++;
				}
				break;
			}
		}
		L2Ptable[lpn] = blk * N_PAGES_PER_BLOCK + page;
		nand_write(blk, page, *write_buffer, temp);
	}
	return;
}

#endif

int calc_cost(int blk){
	return 1;
}


static void garbage_collection(void)

/***************************************
You can add some arguments and change
return type to anything you want
***************************************/
{
	s.gc++;
	int blk_to_erase = -1;;
	int max_val = 0;
	#ifdef COST_BENEFIT
	for (int i = 0; i < N_BLOCKS; i++)
	{

		if (i == spare_block_N)
		{

			continue;
		}

		int cost_of_this = calc_cost(i)
		//	printf("invalid pages per block %d : %d\n", i, cnt);
		if (max_val < cost_of_this)
		{
			max_val = cost_of_this;
			blk_to_erase = i;
		}
	}

	#else
	for (int i = 0; i < N_BLOCKS; i++)
	{
		if (i == spare_block_N)
		{
			continue;
		}

		//	printf("invalid pages per block %d : %d\n", i, cnt);
		if (max_val < invalid_pages_per_block[i])
		{
			max_val = invalid_pages_per_block[i];
			blk_to_erase = i;
		}
	}
	#endif
	invalid_pages_per_block[blk_to_erase] = 0;
	//printf("blk to erase %d / spare page %d\n", blk_to_erase, spare_block_N);
	written_pages[spare_block_N] = 0;
	for (int i = 0; i < N_PAGES_PER_BLOCK; i++)
	{
		if (PPN_invaild[blk_to_erase * N_PAGES_PER_BLOCK + i] == 0)
		{
			int blk = spare_block_N;
			int page = written_pages[blk];
			u32 data;
			u32 spare;
			written_pages[spare_block_N]++;
			nand_read(blk_to_erase, i, &data, &spare);
			nand_write(blk, page, data, spare);
			s.gc_write++;
		//	printf("unsingend = %d %d %d\n", i, data, spare);
			L2Ptable[spare] = blk * N_PAGES_PER_BLOCK + page;
		}
		else
		{
			PPN_invaild[blk_to_erase * N_PAGES_PER_BLOCK + i] = 0;
		}
	}
	nand_erase(blk_to_erase);
	spare_block_N = blk_to_erase;

	written_pages[blk_to_erase] = 0;
	full_page_number--;

	return;
}
