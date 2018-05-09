
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

LOCKBENCH = testperf

OBJS=    testperf.o
SRCS=    testperf.c 

CFLAGS  = -O -I$(INCLUDE) -I$(LOCKINCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

$(LOCKBENCH) :  
	cc $(CFLAGS) $@.o  $(BIN)/wiss.o -o $@

testperf: testperf.o $(BIN)/wiss.o

disk:
	../../bin/format -  hp0a < disk.script

clean:
	rm -f core testperf hp0a
	rm -f $(OBJS) $(EXECS) 

