#
#  Makefile for TEST programs for level 3 of Wiss
#
#

#  remove the # from the front of the following lines to enable
#  the corresponding option during C compilation
TRACEFLAG = -DTRACE
#DEBUGFLAG = -DDEBUG
PROF = -pg

INCLUDE=../../src
L0 = $(INCLUDE)/0
L1 = $(INCLUDE)/1
L2 = $(INCLUDE)/2
L3 = $(INCLUDE)/3
UTIL = $(INCLUDE)/util
MISC = $(INCLUDE)/misc

EXECS= test1 test2 test3 test4 test5 test6 test7 test8 test9\
	test10 test11 test12 test15 test16 \
	test17 test18

OBJS= test1.o test2.o test3.o test4.o test5.o test6.o test7.o test8.o test9.o\
	test10.o test11.o test12.o test13.o test14.o test15.o test16.o\
	test17.o test18.o

SRCS=  test1.c test2.c test3.c test4.c test5.c test6.c test7.c test8.c test9.c\
	test10.c test11.c test12.c test13.c test14.c test15.c test16.c\
	test17.c test18.c

## Debugging flags:
#	LDFLAGS allows sdb access to system routines
#	CFLAGS  retains symbols in test program
CFLAGS  = -g -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

## Non-debugging flags:
#	CFLAGS  invokes a pass to improve object code
#CFLAGS  = -O -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)


Everything : $(EXECS)
	@echo -n >Everything
#
#$(EXECS) : $(INCLUDE)/wiss.o
#	@echo cc $*.o ... -o $*
#	@cc $(INCLUDE)/wiss.o $(CFLAGS) $*.o -o $*


test18 : test18.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test18.o -o test18
test17 : test17.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test17.o -o test17
test16 : test16.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test16.o -o test16
test15 : test15.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test15.o -o test15
test14 : test14.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test14.o -o test14
test13 : test13.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test13.o -o test13
test12 : test12.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test12.o -o test12
test11 : test11.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test11.o -o test11
test10 : test10.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test10.o -o test10
test9 : test9.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test9.o -o test9
test8 : test8.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test8.o -o test8
test7 : test7.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test7.o -o test7
test6 : test6.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test6.o -o test6
test5 : test5.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test5.o -o test5
test4 : test4.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test4.o -o test4
test3 : test3.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test3.o -o test3
test2 : test2.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test2.o -o test2
test1 : test1.o
	cc $(INCLUDE)/wiss.o $(CFLAGS) test1.o -o test1

disk:
	rm -f wiss3
	$(MISC)/format -  wiss3 <disk.script
mdisk :
	@rm -f wiss3.1
	@rm -f wiss3.2
	@rm -f wiss3.3
	$(MISC)/format -  wiss3.1 <disk.script1
	$(MISC)/format -  wiss3.2 <disk.script2
	$(MISC)/format -  wiss3.3 <disk.script3


tape:
	tar cf /dev/rmt8 Makefile test*c disk.script format.count

clean:
	rm -f core
	rm -f $(OBJS) $(EXECS) wiss3
	rm -f wiss3.1 wiss3.2 wiss3.3
