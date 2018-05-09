#
#  Makefile for TEST programs for level 2 of Wiss
#
#

#  remove the # from the front of the following lines to enable
#  the corresponding option during C compilation
TRACEFLAG = -DTRACE
DEBUGFLAG = -DDEBUG
PROF = -p

INCLUDE=../../../wiss
L0 = $(INCLUDE)/0
L1 = $(INCLUDE)/1
L2 = $(INCLUDE)/2
UTIL = $(INCLUDE)/util
MISC = $(INCLUDE)/misc

EXECS= test1 test2 test3 test4 test5 test6 test7 test8 test9\
	test10 test11 test12 test13 test14 test15 test16 test17\
	test20 test21 test22 test23 test24 test25\
	test31 test32 test33 test34 test35

OBJS= test1.o test2.o test3.o test4.o test5.o test6.o test7.o test8.o test9.o\
	test10.o test11.o test12.o test13.o test14.o test15.o test16.o test17.o\
	test20.o test21.o test22.o test23.o test24.o test25.o\
	test31.o test32.o

SRCS= test1.c test2.c test3.c test4.c test5.c test6.c test7.c test8.c test9.c\
	test10.c test11.c test12.c test13.c test14.c test15.c test16.c test17.c\
	test20.c test21.c test22.c test23.c test24.c test25.c\
	test31.c test32.c



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

test32 : test32.o
test31 : test31.o
test25 : test25.o
test24 : test24.o
test23 : test23.o
test22 : test22.o
test21 : test21.o
test20 : test20.o
test17 : test17.o
test16 : test16.o
test15 : test15.o
test14 : test14.o
test13 : test13.o
test12 : test12.o
test11 : test11.o
test10 : test10.o
test9 : test9.o
test8 : test8.o
test7 : test7.o
test6 : test6.o
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
	rm -f $(OBJS) $(EXECS) wiss2
