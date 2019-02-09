# Makefile for shared memory component of System V WiSS

# some make's don't define an assembler or c-preprocessor
AS=	as
CPP=	/usr/local/bin/cpp

# flag for cc and lint
INCLUDE = ..

CFLAGS = -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF) $(READAHEAD) 
#CFLAGS = -I$(INCLUDE) $(TRACEFLAG) $(DEBUGFLAG) $(PROF) $(READAHEAD) $(-EVENTFLAG) 

SM =  sm_final.o sm_init.o semaphore.o latch.o testandset.o

SMC =  sm_final.c sm_init.csemaphore.c latch.c

# Here come the dependencies
#
# first, the thing which depends on everything else
sm.o : $(SM)  # call our module sm.o
	ar cr libsm.a $(SM)
	@echo ld -r ~~~ -o sm.o
	@ld -r $(SM) -o sm.o

# all the object files depend on some header files
$(SM)  : $(INCLUDE)/wiss.h $(INCLUDE)/page.h $(INCLUDE)/sm.h  $(INCLUDE)/latch.h

testandset.o: testandset.S
# this version should work most places.  We use a '.' temporary
# name so that the default '.s -> .o' rule doesn't conflict with us
	$(CPP) testandset.S | grep -v '^#' > .testandset.s
	$(AS) -o $@ .testandset.s
	rm -f .testandset.s

lint : $(SMC) 
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) $(SMC) > lint.out

clean :
	/bin/rm -f *.o
