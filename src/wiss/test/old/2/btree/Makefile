#
#  Makefile for TEST programs for level 2 btree routine
#
#

#  remove the # from the front of the following lines to enable
#  the corresponding option during C compilation
#TRACEFLAG = -DTRACE
#DEBUGFLAG = -DDEBUG -g
PROF = -pg

INCLUDE=../../../src
CFLAGS  = -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

L0 = $(INCLUDE)/0
L1 = $(INCLUDE)/1
L2 = $(INCLUDE)/2
L3 = $(INCLUDE)/3
RECORD = $(INCLUDE)/2/record
BT = $(INCLUDE)/2/btree
UTIL = $(INCLUDE)/util
MISC = $(INCLUDE)/misc

EXECS=	test1	test2   Test5 Test4 Test100 Test3 test3	test4	test5	test6 	test7	test8	test9

OBJS=   test1.o test2.o Test5.o  Test4.o Test100.o Test3.o test3.o	test4.o	test5.o	test6.o	test7.o	test8.o	test9.o

SRCS=   test1.c	test2.c Test5.c Test4.c Test100.c Test3.c test3.c	test4.c	test5.c	test6.c	test7.c	test8.c	test9.c\
	testa.c	testb.c	testc.c	testd.c	teste.c

Everything : $(EXECS)
	@echo -n >Everything

$(EXECS) : $(L0)/wiss0.o $(L1)/wiss1.o $(L2)/wiss2.o $(L3)/wiss3.o \
	$(UTIL)/util.o 
	@echo cc $*.o ... -o $*
	@cc $(CFLAGS) $*.o $(INCLUDE)/wiss.o -o $*

test1 : test1.o
test2 : test2.o
Test5 : Test5.o
Test4 : Test4.o
Test3 : Test3.o
test3 : test3.o
test4 : test4.o
test5 : test5.o
test6 : test6.o
test7 : test7.o
test8 : test8.o
test9 : test9.o
testa : testa.o 
testb : testb.o
testc : testc.o
testd : testd.o
teste : teste.o
Test100: Test100.o
disk:
	rm -f wiss2
	$(MISC)/format -  wiss2 <disk.script

tape:
	tar cf /dev/rmt8 Makefile test*c disk.script format.count

clean:
	rm -f core
	rm -f wiss2
	rm -f $(OBJS) $(EXECS) wiss2b

