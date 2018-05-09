#  Makefile for TEST programs for level 0 of Wiss
#

#  remove the # from the front of the following lines to enable
#  the corresponding option during C compilation
#TRACEFLAG = -DTRACE
#DEBUGFLAG = -DDEBUG

INCLUDE = ../../../wiss
L0 = $(INCLUDE)/0
UTIL = $(INCLUDE)/util
MISC = $(INCLUDE)/misc
CC = gcc

EXECS = test1 test2 test3 test4 test5 

OBJS = test1.o test2.o test3.o test4.o test5.o 

SRCS = test1.c test2.c test3.c test4.c test5.c 

## Debugging flags:
#	LDFLAGS allows sdb access to system routines
#	CFLAGS  retains symbols in test program
LDFLAGS = -lg
#CFLAGS  = -g -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG)

## Non-debugging flags:
#	CFLAGS  invokes a pass to improve object code
CFLAGS  = -O -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG)

Everything : $(EXECS)
	@echo -n >Everything

$(EXECS) : $(L0)/wiss0.o  $(UTIL)/util.o
	@echo gcc $*.o ~~~ -o $*
	${CC} $(LDFLAGS) $*.o $(L0)/wiss0.o  $(UTIL)/util.o -o $*

test5 : test5.o
test4 : test4.o
test3 : test3.o
test2 : test2.o
test1 : test1.o

disk:
	rm -f wiss0
	$(MISC)/format - wiss0 <disk.script

tape:
	tar cf /dev/rmt8 Makefile test*c disk.script format.count

clean:
	rm -f a.out core
	rm -f $(OBJS) $(EXECS) wiss0
