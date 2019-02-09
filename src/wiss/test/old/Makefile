#
#  Makefile for TEST programs 
#

#  remove the # from the front of the following lines to enable
#  the corresponding option during C compilation
TRACEFLAG = -DTRACE
#DEBUGFLAG = -DDEBUG
#PROF = -p

INCLUDE = ../src
MISC = $(INCLUDE)/misc

EXECS= test1 test2 test3 

OBJS= test1.o test2.o test3.o 

SRCS=  test1.c test2.c test3.c 

## Debugging flags:
#	LDFLAGS allows sdb access to system routines
#	CFLAGS  retains symbols in test program
#LDFLAGS = -lg
CFLAGS  = -g -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

## Non-debugging flags:
#	CFLAGS  invokes a pass to improve object code
#CFLAGS  = -O -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

Everything : $(EXECS)
	cc $(LDFLAGS) $(CFLAGS) test1.o $(INCLUDE)/wiss.o -o test1
	cc $(LDFLAGS) $(CFLAGS) test2.o $(INCLUDE)/wiss.o -o test2
	cc $(LDFLAGS) $(CFLAGS) test3.o $(INCLUDE)/wiss.o -o test3
#	@echo -n >Everything

#$(EXECS) : $(INCLUDE)/wiss.o
#	cc $*.o ... -o $*
#	cc $(LDFLAGS) $(CFLAGS) $*.o $(INCLUDE)/wiss.o -o $*
################################################################
#$(EXECS) : $(INCLUDE)/wiss.o
#	@echo cc $*.o ... -o $*
#	@cc $(LDFLAGS) $(CFLAGS) $*.o $(INCLUDE)/wiss.o -o $*
################################################################
test3 : test3.o
test2 : test2.o
test1 : test1.o

disk:
	rm -f wiss_disk
	$(MISC)/format - wiss_disk <disk.script

clean:
	rm -f core
	rm -f $(OBJS) $(EXECS) wiss_disk
