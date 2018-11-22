#---------------------------------------------------------
#
# NAND Simulator
#
# v3 by Dong-Yun Lee (April 20, 2017)
# v2 by Jin-Soo Kim (July 30, 2005)
#---------------------------------------------------------

PREFIX		=
CC			= $(PREFIX)gcc
CXX			= $(PREFIX)g++
WORKLOAD	= 	 -DMULTI_HOT_COLD #-DHOT_COLD  
				 
				# if -DHOT_COLD is not set, WORKLOAD will be random as a Project #2

FTL			= # -DCOST_BENEFIT #-DMULTI_STREAM 

			# if both -DMULTI_STREAM and -DCOST_BENEFIT is not set, FTL should be a greedy FTL

CFLAGS		= -g -O2 -Wall $(WORKLOAD) $(FTL)
LIBS		= 
RM			= rm
TAR			= tar

TARGET		= pm_sim
CSRCS		= pm_sim.c nand.c pm.c
CXXSRCS		= 
HEADERS		= nand.h pm.h
OBJS		= $(CSRCS:.c=.o) $(CXXSRCS:.cpp=.o)

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS) 

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o: $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

tar:
	$(RM) -f $(TARGET).tar.gz
	$(TAR) cvzf $(TARGET).tar.gz $(CSRCS) $(HEADERS) Makefile
	ls -l $(TARGET).tar.gz

clean:
	$(RM) -f $(TARGET) $(TARGET).tar.gz $(OBJS) *~
