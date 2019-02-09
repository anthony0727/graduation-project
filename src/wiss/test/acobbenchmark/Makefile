
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

O2BENCH = queries build 

OBJS=    build.o queries.o 
SRCS=    build.c queries.c 

CFLAGS  = -g -I$(INCLUDE) -I$(LOCKINCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

o2benchmark : $(O2BENCH)

$(O2BENCH) :  
	cc $(CFLAGS) $@.o  $(BIN)/wiss.o -o $@

build: build.o build.h $(BIN)/wiss.o
queries: queries.o build.h $(BIN)/wiss.o

disk: 
	../../bin/format -  hp0a < disk.script

clean:
	rm -f core out hp0a build queries
	rm -f $(OBJS) $(EXECS) 

