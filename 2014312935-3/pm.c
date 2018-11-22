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
static long now(void)
{
	return s.host_write + s.gc_write;
}
#endif

int *L2Ptable;
int *written_pages;
int *PPN_invaild;
int *invalid_pages_per_block;
int full_page_number;
int spare_block_N;

int *block_age;
int *stream_id;
int now_write_stream;

static void
garbage_collection(void);

void ftl_open(void)
{
	int a = nand_init(N_BLOCKS, N_PAGES_PER_BLOCK);
	if (a != 0)
	{
		return;
	}
	L2Ptable = (int *)malloc(sizeof(int) * N_USER_BLOCKS * N_PAGES_PER_BLOCK);
	written_pages = (int *)malloc(sizeof(int) * N_BLOCKS);
	PPN_invaild = (int *)malloc(sizeof(int) * N_BLOCKS * N_PAGES_PER_BLOCK);
	invalid_pages_per_block = (int *)malloc(sizeof(int) * N_BLOCKS);
	block_age = (int *)malloc(sizeof(int) * N_BLOCKS);
	stream_id = (int *)malloc(sizeof(int) * N_BLOCKS);

	for (int i = 0; i < N_USER_BLOCKS * N_PAGES_PER_BLOCK; i++)
	{
		L2Ptable[i] = -1;
	}
	for (int i = 0; i < N_BLOCKS; i++)
	{
		written_pages[i] = 0;
		block_age[i] = 0;
		stream_id[i] = -1;
	}

	for (int i = 0; i < N_BLOCKS * N_PAGES_PER_BLOCK; i++)
	{
		PPN_invaild[i] = 0;
	}

	spare_block_N = N_BLOCKS - 1;

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
void ftl_write(int streamid, long lpn, u32 *write_buffer)
{

	now_write_stream = streamid;
	//printf("stream %d \n", streamid);
	int full_cnt = 0;

	for (int i = 0; i < N_BLOCKS; i++)
	{
		if (written_pages[i] != N_PAGES_PER_BLOCK && i != spare_block_N && (stream_id[i] == -1 || stream_id[i] == streamid))
		{
			full_cnt++;
		}
	}
	if (full_cnt == 0)
	{

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
				if (stream_id[i] == -1)
					stream_id[i] = streamid;
				//printf("input strea %d ", streamid);
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

		invalid_pages_per_block[old_page_cnt / N_PAGES_PER_BLOCK]++;
#ifdef COST_BENEFIT
		block_age[old_page_cnt / N_PAGES_PER_BLOCK] = now();
#endif

		for (int i = 0; i < N_BLOCKS; i++)
		{
			if (written_pages[i] != N_PAGES_PER_BLOCK && i != spare_block_N && (stream_id[i] == -1 || stream_id[i] == streamid))
			{
				blk = i;
				page = written_pages[i];
				if (stream_id[i] == -1)
					stream_id[i] = streamid;
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

	for (int i = 0; i < N_BLOCKS; i++)
	{
		if (written_pages[i] == N_PAGES_PER_BLOCK)
		{
			full_cnt++;
		}
	}
	if (full_cnt == N_BLOCKS - 1)
	{

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
		invalid_pages_per_block[old_page_cnt / N_PAGES_PER_BLOCK]++;
#ifdef COST_BENEFIT
		block_age[old_page_cnt / N_PAGES_PER_BLOCK] = now();
#endif

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
#ifdef COST_BENEFIT
float calc_cost(int blk)
{
	float cost_ben;
	float u = (float)(N_PAGES_PER_BLOCK - invalid_pages_per_block[blk]) / (float)N_PAGES_PER_BLOCK;

	float age = now() - (block_age[blk]);
	cost_ben = ((1 - u) / u);
	cost_ben = cost_ben * age / 2;

	return cost_ben;
}
#endif

int find()
{
	int max1 = 0;
	int max2 = 0;
	int victim1, victim2;
	for (int i = 0; i < N_BLOCKS; i++)
	{
		if (invalid_pages_per_block[i] > max1)
		{
			victim1 = i;
			max1 = invalid_pages_per_block[i];
		}
	}
	for (int i = 0; i < N_BLOCKS; i++)
	{
		if (i == victim1 || stream_id[victim1] != stream_id[i])
		{
			continue;
		}
		if (invalid_pages_per_block[i] > max2)
		{
			victim2 = i;
			max2 = invalid_pages_per_block[i];
		}
	}

	int valid1 = N_PAGES_PER_BLOCK - invalid_pages_per_block[victim1];
	int valid2 = N_PAGES_PER_BLOCK - invalid_pages_per_block[victim2];
	//printf("vic1 %d vic2 %d  streamid %d %d  vlid %d\n", victim1, victim2, stream_id[victim1], stream_id[victim2], valid1 + valid2);

	if (valid2 + valid1 <= N_PAGES_PER_BLOCK)
	{
		invalid_pages_per_block[victim1] = 0;
		invalid_pages_per_block[victim2] = 0;
		written_pages[spare_block_N] = 0;
		for (int i = 0; i < N_PAGES_PER_BLOCK; i++)
		{
			if (PPN_invaild[victim1 * N_PAGES_PER_BLOCK + i] == 0)
			{
				int blk = spare_block_N;
				int page = written_pages[blk];
				u32 data;
				u32 spare;
				written_pages[spare_block_N]++;
				nand_read(victim1, i, &data, &spare);
				nand_write(blk, page, data, spare);
				s.gc_write++;
				//	printf("unsingend = %d %d %d\n", i, data, spare);
				L2Ptable[spare] = blk * N_PAGES_PER_BLOCK + page;
			}
			else
			{
				PPN_invaild[victim1 * N_PAGES_PER_BLOCK + i] = 0;
			}
		}
		for (int i = 0; i < N_PAGES_PER_BLOCK; i++)
		{
			if (PPN_invaild[victim2 * N_PAGES_PER_BLOCK + i] == 0)
			{
				int blk = spare_block_N;
				int page = written_pages[blk];
				u32 data;
				u32 spare;
				written_pages[spare_block_N]++;
				nand_read(victim2, i, &data, &spare);
				nand_write(blk, page, data, spare);
				s.gc_write++;
				//	printf("unsingend = %d %d %d\n", i, data, spare);
				L2Ptable[spare] = blk * N_PAGES_PER_BLOCK + page;
			}
			else
			{
				PPN_invaild[victim2 * N_PAGES_PER_BLOCK + i] = 0;
			}
		}
		nand_erase(victim1);
		nand_erase(victim2);

		if (valid1 + valid2 != 0)
			stream_id[spare_block_N] = stream_id[victim1];
		else
			stream_id[spare_block_N] = -1;
		stream_id[victim1] = -1;
		stream_id[victim2] = -1;
		spare_block_N = victim2;

		written_pages[victim2] = 0;
		written_pages[victim1] = 0;
		return 0;
	}
	else
	{
		invalid_pages_per_block[victim1] = 0;

		written_pages[spare_block_N] = 0;
		for (int i = 0; i < N_PAGES_PER_BLOCK; i++)
		{
			if (PPN_invaild[victim1 * N_PAGES_PER_BLOCK + i] == 0)
			{
				int blk = spare_block_N;
				int page = written_pages[blk];
				u32 data;
				u32 spare;
				written_pages[spare_block_N]++;
				nand_read(victim1, i, &data, &spare);
				nand_write(blk, page, data, spare);
				s.gc_write++;
				//	printf("unsingend = %d %d %d\n", i, data, spare);
				L2Ptable[spare] = blk * N_PAGES_PER_BLOCK + page;
			}
			else
			{
				PPN_invaild[victim1 * N_PAGES_PER_BLOCK + i] = 0;
			}
		}
		for (int i = 0; i < N_PAGES_PER_BLOCK; i++)
		{
			if (written_pages[spare_block_N] == N_PAGES_PER_BLOCK)
				break;
			if (PPN_invaild[victim2 * N_PAGES_PER_BLOCK + i] == 0)
			{
				int blk = spare_block_N;
				int page = written_pages[blk];
				u32 data;
				u32 spare;
				written_pages[spare_block_N]++;
				nand_read(victim2, i, &data, &spare);
				nand_write(blk, page, data, spare);
				s.gc_write++;
				//	printf("unsingend = %d %d %d\n", i, data, spare);
				L2Ptable[spare] = blk * N_PAGES_PER_BLOCK + page;

				PPN_invaild[victim2 * N_PAGES_PER_BLOCK + i] = 1;
				invalid_pages_per_block[victim2]++;
			}
		}
		nand_erase(victim1);

		stream_id[spare_block_N] = stream_id[victim1];

		stream_id[victim1] = -1;
		spare_block_N = victim1;

		written_pages[victim1] = 0;

		return 1;
	}
}

#ifdef MULTI_STREAM

static void garbage_collection(void)
{
	s.gc++;
	int blk_to_erase = -1;
	int max_val2 = 0;
	/*for (int i = 0; i < N_BLOCKS; i++)
	{
		if (i == spare_block_N)
		{
			continue;
		}
		if (stream_id[i] != now_write_stream)
		{
			continue;
		}
		//printf("%d ", stream_id[i]);

		//	printf("invalid pages per block %d : %d\n", i, cnt);
		if (max_val2 < invalid_pages_per_block[i])
		{
			max_val2 = invalid_pages_per_block[i];
			blk_to_erase = i;
		}
	}
	if (blk_to_erase != -1)
	{
		invalid_pages_per_block[blk_to_erase] = 0;

		written_pages[spare_block_N] = 0;
		stream_id[blk_to_erase] = -1;
		stream_id[spare_block_N] = -1;
		//printf("blk to erase %d %d\n", blk_to_erase, spare_block_N);
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

		block_age[spare_block_N] = block_age[blk_to_erase];
		spare_block_N = blk_to_erase;
		block_age[blk_to_erase] = 0;

		written_pages[blk_to_erase] = 0;
		full_page_number--;

		return;
	}
	else
	{*/
	//printf("Asdf ");
	while (1)
	{
		int check = find();
		if (check == 0)
		{
			break;
		}
	}
	return;
}

#else
static void garbage_collection(void)

/***************************************
You can add some arguments and change
return type to anything you want
***************************************/
{

	s.gc++;
	int blk_to_erase = -1;
	//printf("stream id ==%d\n", now_write_stream);

#ifdef COST_BENEFIT
	float max_val = 0;
	for (int i = 0; i < N_BLOCKS; i++)
	{

		if (i == spare_block_N)
		{

			continue;
		}

		float cost_of_this = calc_cost(i);
		//printf("%f\n", cost_of_this);
		//	printf("invalid pages per block %d : %d\n", i, cnt);
		if (max_val < cost_of_this)
		{
			max_val = cost_of_this;
			blk_to_erase = i;
		}
	}

#else
	int max_val2 = 0;
	for (int i = 0; i < N_BLOCKS; i++)
	{
		if (i == spare_block_N)
		{
			continue;
		}
		//printf("%d ", stream_id[i]);

		//	printf("invalid pages per block %d : %d\n", i, cnt);
		if (max_val2 < invalid_pages_per_block[i])
		{
			max_val2 = invalid_pages_per_block[i];
			blk_to_erase = i;
		}
	}
	//printf("asdfasf : %d", max_val);
#endif

	/*
	int stream1 = 0;
	int stream2 = 0;
	int stream3 = 0;
	for (int i = 0; i < N_BLOCKS; i++)
	{
		if (stream_id[i] == 0)
			stream1++;
		else if (stream_id[i] == 3)
			stream2++;
		else if (stream_id[i] == 6)
			stream3++;
	}
	int check3 = 0;
	for (int i = 0; i < N_BLOCKS; i++)
	{
		if (stream_id[i] == now_write_stream)
		{
			if (invalid_pages_per_block[i] != 0)
				check3++;
		}
	}

	if (blk_to_erase == -1)
	{
		printf("stream : %d %d %d now : %d \n", stream1, stream2, stream3, check3);
	}
	int max_val3 = 0;
	if (blk_to_erase == -1)
	{
		//printf("%d no matched\n", now_write_stream);
		for (int i = 0; i < N_BLOCKS; i++)
		{
			if (i == spare_block_N)
			{
				continue;
			}

			//	printf("invalid pages per block %d : %d\n", i, cnt);
			if (max_val3 < invalid_pages_per_block[i])
			{
				max_val3 = invalid_pages_per_block[i];
				blk_to_erase = i;
			}
		}
		if (stream_id[blk_to_erase] == 6)
		{
			printf("s %d %d \n", now_write_stream, stream_id[blk_to_erase]);
		}
	}*/
	//printf("%d", blk_to_erase);
	invalid_pages_per_block[blk_to_erase] = 0;

	written_pages[spare_block_N] = 0;
	stream_id[blk_to_erase] = -1;
	stream_id[spare_block_N] = -1;
	//printf("blk to erase %d %d\n", blk_to_erase, spare_block_N);
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

	block_age[spare_block_N] = block_age[blk_to_erase];
	spare_block_N = blk_to_erase;
	block_age[blk_to_erase] = 0;

	written_pages[blk_to_erase] = 0;
	full_page_number--;

	return;
}

#endif
