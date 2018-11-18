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

CFLAGS		= -g -O2 -Wall -std=c99 $(WORKLOAD) $(FTL)
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
