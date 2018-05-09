#
#  Makefile for TEST programs for level 1 of Wiss

#  remove the # from the front of the following lines to enable
#  the corresponding option during C compilation
TRACEFLAG = -DTRACE
DEBUGFLAG = -DDEBUG

INCLUDE=../../src
L0 = $(INCLUDE)/0
L1 = $(INCLUDE)/1
UTIL = $(INCLUDE)/util
MISC = $(INCLUDE)/misc

EXECS=test1 test2 test3  test4

OBJS= test1.o test2.o test3.o test4.o

SRCS= test1.c test2.c test3.c test4.c

## Debugging flags:
#	LDFLAGS allows sdb access to system routines
#	CFLAGS  retains symbols in test program
LDFLAGS = -lg
CFLAGS  = -g -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG)

## Non-debugging flags:
#	CFLAGS  invokes a pass to improve object code
#CFLAGS  = -O -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG)

test2 : test2.o
	cc -g -o test2 -I$(INCLUDE) ../../src/wiss.o test2.o

test1 : test1.o
	cc -g -o test1 -I$(INCLUDE) ../../src/wiss.o test1.o

test2.o: test2.c
	cc -c -g test2.c -I$(INCLUDE)

test1.o: test1.c
	cc -c -g test1.c -I$(INCLUDE)

#Everything : $(EXECS)
#	@echo -n >Everything

#$(EXECS) : $(L0)/wiss0.o $(L1)/wiss1.o $(UTIL)/util.o 
#	@echo cc $*.o ~~~ -o $*
#	@cc $(LDFLAGS) $*.o $(L0)/wiss0.o $(L1)/wiss1.o $(UTIL)/util.o -o $*


disk:
	@/bin/rm -f wiss1
	$(MISC)/format - wiss1 <disk.script

tape:
	tar cf /dev/rmt8 Makefile test*c disk.script format.count

clean:
	rm -f a.out core 
	rm -f $(EXECS) $(OBJS) wiss1
