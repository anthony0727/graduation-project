#
#  Makefile for TEST programs for level 2 of Wiss (hash indices)
#

TRACEFLAG = -DTRACE
#DEBUGFLAG = -DDEBUG
PROF = -p

INCLUDE=../../../src
L0 = $(INCLUDE)/0
L1 = $(INCLUDE)/1
L2 = $(INCLUDE)/2
UTIL = $(INCLUDE)/util
MISC = $(INCLUDE)/misc

EXECS= test1 test2 test3 test4 test5 

OBJS= test1.o test2.o test3.o test4.o test5.o 

SRCS= test1.c test2.c test3.c test4.c test5.c 

## Debugging flags:
#	LDFLAGS allows sdb access to system routines
#	CFLAGS  retains symbols in test program
#LDFLAGS = -lg
CFLAGS  = -g -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

## Non-debugging flags:
#	CFLAGS  invokes a pass to improve object code
#CFLAGS  = -O -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

Everything : $(EXECS)
	@echo -n >Everything

$(EXECS) : $(L0)/wiss0.o $(L1)/wiss1.o $(L2)/wiss2.o \
	$(UTIL)/util.o
	@echo cc $*.o ... -o $*
	@cc $(CFLAGS) $*.o $(L0)/wiss0.o $(L1)/wiss1.o $(L2)/wiss2.o $(UTIL)/util.o -o $*

test5 : test5.o
test4 : test4.o
test3 : test3.o
test2 : test2.o
test1 : test1.o

disk:
	rm -f wiss2
	$(MISC)/format -  wiss2 <disk.script

clean:
	rm -f core
	rm -f $(OBJS) $(EXECS) wiss2 mon.out
