//----------------------------------------------------------
//
// Project #3 : Other Page mapping FTLs
// 	- Embedded Systems Design, ICE3028 (Spring, 2017)
//
// Apr. 20, 2017.
//
// Dong-Yun Lee (dongyun.lee@csl.skku.edu)
// Jin-Soo Kim (jinsookim@skku.edu)
// Computer Systems Laboratory
// Sungkyunkwan University
// http://csl.skku.edu/ICE3028S17
//
//---------------------------------------------------------

#include <stdio.h>
#include <assert.h>
#include "pm.h"



char *bit2cap (int b)
{
	static char *unit[] = {"", "K", "M", "G", "T", "P", "E"};
	static char buf[32];

	int u = b / 10;

   	if (u > 6)
        return "<Too Big>";

	sprintf (buf, "%d%s", (1 << (b - u*10)), unit[u]);
	return buf;
}

void sim_init (void)
{
  	s.gc = 0;
   	s.host_write = 0;
   	s.gc_write = 0;
	srand (0);

	ftl_open();
}

void show_info (void)
{
	printf ("SSD capacity:\t\t%sB\n", bit2cap (SSD_SHIFT));
	printf ("Page size:\t\t%sB\n", bit2cap (PAGE_SHIFT));
	printf ("Pages / Block:\t\t%d pages\n", N_PAGES_PER_BLOCK);
   	printf ("Block size:\t\t%sB\n", bit2cap (PAGES_PER_BLOCK_SHIFT + PAGE_SHIFT));
	printf ("OP ratio:\t\t%d%%\n", OP_RATIO);
	printf ("Physical Blocks:\t%s (%d)\n", bit2cap(BLOCKS_SHIFT), N_BLOCKS);
   	printf ("User Blocks:\t\t%d\n", N_USER_BLOCKS);
	printf ("OP Blocks:\t\t%d\n", N_OP_BLOCKS);
	printf ("PPNs:\t\t\t%s (%d)\n", bit2cap (PPN_SHIFT), N_PPNS);
	printf ("LPNs:\t\t\t%d\n", N_LPNS);
   	printf ("Total runs:\t\tx%d\n", N_RUNS);
	printf ("Actual capacity:\t%ld Bytes\n\n", (long) N_LPNS * N_PAGE_SIZE);

#ifdef MULTI_HOT_COLD
	printf("Workload : Multi Hot/Cold\n");
#elif HOT_COLD
	printf("Workload : Hot/Cold\n");
#else 
	printf("Workload : Random\n");
#endif

#ifdef MULTI_STREAM
	printf ("FTL : Multi-Stream policy\n");
#elif COST_BENEFIT
	printf ("FTL : Cost-Benefit policy\n");
#else
	printf ("FTL : Greedy policy\n");
#endif

}
long dy_hot, dy_cold = 0;


#ifdef MULTI_STREAM
long get_lpn(int *streamid)
#else
long get_lpn(void)
#endif

{
	long lpn;
	double prob;	

#ifdef MULTI_HOT_COLD
	prob = random() % 100;

	if (prob < HOT_RATIO)
	{
	// HOT
		lpn = random () % HOT_LPN;
	#ifdef MULTI_STREAM
		*streamid = HOT_STREAMID;
	#endif
	}
	else if (prob < HOT_RATIO + WARM_RATIO)
	{
	// WARM
		lpn = HOT_LPN + (random () % WARM_LPN); 
	#ifdef MULTI_STREAM
		*streamid = WARM_STREAMID;
	#endif
	}
	else 
	{
	// COLD
		lpn = HOT_LPN + WARM_LPN + (random () % COLD_LPN);
	#ifdef MULTI_STREAM
		*streamid = COLD_STREAMID;
	#endif
	}

#elif HOT_COLD
	prob = random () % 100;
	if (prob < HOT_RATIO) 
	{
	// HOT
	//	printf("HOT\n");
		lpn = random () % HOT_LPN;
	#ifdef MULTI_STREAM
		*streamid = HOT_STREAMID;
	#endif
	} 
	else 
	{
	// COLD
	//	printf("COLD\n");
		lpn = HOT_LPN + (random () % COLD_LPN);
	#ifdef MULTI_STREAM
		*streamid = COLD_STREAMID;
	#endif
	}

#else
	lpn = random () % N_LPNS;
	#ifdef MULTI_STREAM
	*streamid = HOT_STREAMID;
	#endif
#endif

	return lpn;
}


void sim()
{
	long lpn;
	u32 write_buffer;
	u32 read_buffer;

	while (s.host_write < LPN_COUNTS)
	{

#ifdef MULTI_STREAM
		
		int streamid;
		lpn = get_lpn (&streamid); // If it's not multi-stream ssd, just ignore streamid
#else
		lpn = get_lpn ();
#endif
		write_buffer = (u32)lpn;

#ifdef MULTI_STREAM
		ftl_write(streamid, lpn, &write_buffer);
#else
		ftl_write(lpn, &write_buffer);
#endif

		ftl_read(lpn, &read_buffer);

		assert(read_buffer == (u32)lpn);

		s.host_write++;
		if (s.host_write % N_LPNS == 0) {
			printf ("[Run %d] host %ld, valid page copy %ld, GC# %d, WAF=%.2f\n",
					(int) s.host_write / N_LPNS, 
                    			s.host_write, s.gc_write, s.gc, 
					(double) (s.host_write + s.gc_write) / (double) s.host_write);
		}
	}
}

void show_stat (void)
{
	printf ("\nResults ------\n");
	printf ("Host writes:\t\t%ld\n", s.host_write);
	printf ("GC writes:\t\t%ld\n", s.gc_write);
	printf ("Number of GCs:\t\t%d\n", s.gc);
	printf ("Valid pages per GC:\t%.2f pages\n", (double) s.gc_write / (double) s.gc);
	printf ("WAF:\t\t\t%.2f\n", (double) (s.host_write + s.gc_write) / (double) s.host_write);
}

int main (void)
{
	sim_init ();
	show_info ();
	sim ();
	show_stat ();
	return 0;
}
