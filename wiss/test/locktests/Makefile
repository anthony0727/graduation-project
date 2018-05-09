#
#
#

#  remove the # from the front of the following lines to enable
#  the corresponding option during C compilation
#TRACEFLAG = -DTRACE
#DEBUGFLAG = -DDEBUG
#PROF = -p

INCLUDE=../../wiss
LOCKINCLUDE=../../wiss/LM
UTIL = $(INCLUDE)/util
MISC = $(INCLUDE)/misc
BIN = ../../bin

TESTS=	test1 test2 test3 test4 

OBJS=   mantester test1.o test2.o test3.o test4.o 

SRCS=   mantester test1.c test2.c test3.c test4.c

CFLAGS  = -g -I$(INCLUDE) -I$(LOCKINCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

all : $(TESTS) mantester
	@echo -n >Everything

$(TESTS) : $(BIN)/wiss.o 
	cc $(CFLAGS) $@.o $(BIN)/wiss.o -o $@

mantester : $(BIN)/wiss.o 
	cc $(CFLAGS) -o mantester mantester.c sgtty.c $(BIN)/wiss.o

test1 : test1.o
test2 : test2.o
test3 : test3.o
test4 : test4.o
mantester : mantester.o

disk:
	$(BIN)/format -  hp0a < disk.script

clean:
	rm -f core *.o hp0a
	rm -f $(OBJS) $(TESTS) 

