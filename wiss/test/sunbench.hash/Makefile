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

EXECS=	build lookup traverse insert 

OBJS=   build.o lookup.o traverse.o insert.o

SRCS=   build.c lookup.c traverse.c insert.c

CFLAGS  = -g -I$(INCLUDE) -I$(LOCKINCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF)

all : $(EXECS)
	@echo -n >Everything

$(EXECS) : $(BIN)/wiss.o 
#	cc $(CFLAGS) $@.o $(BIN)/wiss.o -o $@.mono
	cc $(CFLAGS) $@.o $(BIN)/wiss.o -o $@


build : build.o
lookup : lookup.o
traverse : traverse.o
insert : insert.o
$(OBJS): data.h

disk:
	$(BIN)/format -  hp0a < disk.script


clean:
	rm -f core hp0a SunBench.small
	rm -f $(OBJS) $(EXECS) *.mono

