.KEEP_STATE:

LM=		..
WISS = 		../..
LMl= 		$(LM)/lockmanager
LOCK=           ..

#CFLAGS = -I$(WISS) $(DEBUGFLAG) $(TRACEFLAG) $(PROF) -DLMTRACE 
CFLAGS = -I$(LM) -I$(WISS) -I$(LOCK) $(DEBUGFLAG) $(TRACEFLAG) $(PROF) $(HIER) 

# the INNER files are lower-level routines
INNER = $(LMl)/LM_event.o $(LMl)/release.o $(LMl)/alloclock.o \
	$(LMl)/create.o $(LMl)/initlock.o $(LMl)/locker.o 

INNERC = $(LMl)/LM_event.c $(LMl)/release.c $(LMl)/alloclock.c \
	$(LMl)/create.c $(LMl)/initlock.c $(LMl)/locker.c

# the OUTER files are seen by the outside world
OUTER =  trans.o $(LMl)/info.o  
OUTERC = trans.c $(LMl)/info.c

# Here come the dependencies
# first, the thing which depends on everything else
LockManager.o : $(INNER) $(OUTER) 
	ld -r $(INNER) $(OUTER) -o LockManager.o

EXECS:
	echo $(INNERC) $(OUTERC) >> ../EXECS

lint : $(INNERC) $(OUTERC)
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) $(INNERC) $(OUTERC) >lint.out

$(INNER): $(WISS)/sm.h $(WISS)/wiss.h $(WISS)/page.h $(LM)/lockquiz.h \
	$(LM)/LM_macro.h  $(WISS)/locktype.h

ens:
	enscript -2r -G -P7a $(INNERC)

clean :
	/bin/rm -f *.o 

