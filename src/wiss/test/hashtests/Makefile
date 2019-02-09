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

EXECS=	test1 test2 test3 test4 test5

OBJS=   test1.o test2.o test3.o test4.o test5.o

SRCS=   test1.c test2.c test3.c test4.c test5.c

CFLAGS  = -g -I$(INCLUDE) -I$(LOCKINCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

all : $(EXECS)
	@echo -n >Everything

$(EXECS) : $(BIN)/wiss.o 
	cc $(CFLAGS) $@.o $(BIN)/wiss.o -o $@

test1 : test1.o
test2 : test2.o
test3 : test3.o
test4 : test4.o
test5 : test5.o

disk:
	$(BIN)/format -  hp0a < disk.script

clean:
	rm -f core
	rm -f $(OBJS) $(EXECS) out hp0a

