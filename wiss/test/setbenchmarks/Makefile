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

EXECS=	loaddb loaddbsets scan scansets setindex setindex2 setsort  setsortx setx test multi

OBJS=   loaddb.o scan.o scansets.o loaddbsets.o s setindex.o setindex2.o setsort.o setsortx.o setx.o test.o multi.o

SRCS=   loaddb.c scan.c scansets.c loaddbsets.c setindex.c setindex2.c setsort.c setsortx.c setx.c test.c multi.c

CFLAGS  = -g -I$(INCLUDE) -I$(LOCKINCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

all : $(EXECS)
	@echo -n >Everything

$(EXECS) : $(BIN)/libWiSS.a 
	cc $(CFLAGS) $@.o -L$(BIN) -lWiSS -o $@

$(OBJS) : benchmark.h

test : test.o
multi : multi.o
loaddb : loaddb.o
loaddbsets : loaddbsets.o
scan: scan.o
scansets: scansets.o
setindex: setindex.o
setindex2: setindex2.o
setsort: setsort.o
setsortx: setsortx.o
setx: setx.o

disk:
	$(BIN)/format -  hp0a < disk.script

clean:
	rm -f core hp0a out
	rm -f $(OBJS) $(EXECS) 

