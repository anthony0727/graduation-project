# Makefile for level 1 of WiSS

# flag for cc and lint
INCLUDE = ..

CFLAGS = -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF) $(READAHEAD) 
#CFLAGS = -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF) $(READAHEAD) $(-EVENTFLAG) 

# the BUFFER define level 1
BUFFER = bfopenfile.o bfclosefile.o bfflushbuf.o \
	bfgetbuf.o bffreebuf.o bfreadbuf.o bfdiscard.o \
	bfsetdirty.o bfdismount.o bferror.o bfinit.o bffinal.o \
	bfinvalidate.o BFhash.o BFallocbuf.o BFdumptables.o bffindbuf.o \
	bfunpin.o bfpin.o

BUFFERC =  bfopenfile.c bfclosefile.c bfflushbuf.c \
	bfgetbuf.c bffreebuf.c bfreadbuf.c bfdiscard.c \
	bfsetdirty.c bfdismount.c bferror.c bfinit.c bffinal.c \
	bfinvalidate.c BFhash.c BFallocbuf.c BFdumptables.c bffindbuf.c \
	bfunpin.c bfpin.c

# Here come the dependencies
#
# first, the thing which depends on everything else
wiss1.o : $(BUFFER)  # call our module wiss1.o
	@echo ld -r ~~~ -o wiss1.o
	@ld -r $(BUFFER) -o wiss1.o

# all the object files depend on some header files
$(BUFFER)  : $(INCLUDE)/wiss.h $(INCLUDE)/bf_error.h $(INCLUDE)/bf.h\
	 $(INCLUDE)/page.h $(INCLUDE)/sm.h

bferror.o : bferror.i
bferror.i : bferror.m4
	m4 $(INCLUDE)/handler.m4 bferror.m4 > bferror.i

$(INCLUDE)/bf_error.h : bferror.m4
	m4 $(INCLUDE)/defs.m4 bferror.m4 > $(INCLUDE)/bf_error.h

tags : $(BUFFERC) 
	@ctags $(BUFFERC) 

lint : $(BUFFERC) 
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) $(BUFFERC) > lint.out

clean :
	/bin/rm -f *.o
