# Makefile for WiSS utility library

# flag for cc and lint
INCLUDE = -I..

# This file uses the default rules for transforming .c into .o files:
#	cc $(CFLAGS) file
# The BARE flag should be on when the program is to run on a bare VAX
# 
#DEBUGFLAG = -DDEBUG
#TRACEFLAG = -DTRACE
#PROF = -p

# compile only, keep extra info for SDB
#CFLAGS = -g $(INCLUDE) $(DEBUGFLAG) $(TRACEFLAG) $(PROF) 
CFLAGS = -O $(INCLUDE) $(DEBUGFLAG) $(TRACEFLAG) $(PROF) 

SOURCES = bit.c traceflags.c data.c suffixname.c sys.c shmalloc.c
OBJECTS = bit.o traceflags.o data.o suffixname.o sys.o shmalloc.o

# the utility library everybody needs
util.o : $(OBJECTS) $(TRACEFLAGS) $(DEBUGFLAGS) $(ASSEMO)
	ld -r $(OBJECTS) $(ASSEMO) -o util.o
	chmod -x util.o

# all the object files depend on some header files
$(OBJECTS) : ../wiss.h ../io_error.h ../page.h ../io.h ../sm.h

printf : printf.o putchar.o fltcvt.o
	ld -r -o printf printf.o putchar.o fltcvt.o

dumplog : dumplog.c
	cc $(CFLAGS) dumplog.c -o dumplog

lint : $(SOURCES)
	@echo lint ~~~ '>lint.out'
	@lint $(INCLUDE) $(SOURCES) >lint.out

.s.o:
	/lib/cpp -P $*.s > $*.cpp
	as -o $*.o $*.cpp
	-ld -x -r -o tmp$$$$ $*.o; mv tmp$$$$ $*.o
	rm -f $*.cpp

clean:
	/bin/rm *.o

#
