.KEEP_STATE:

LM=		..
WISS =		../..
LMd=		$(LM)/deadlock
LOCK=		..

CFLAGS = -I$(LM) -I$(WISS) -I$(LOCK) $(DEBUGFLAG) $(TRACEFLAG) $(PROF) 
#CFLAGS = -I$(INCLUDE) -I$(WISS) $(DEBUGFLAG) $(TRACEFLAG) $(PROF) -DLMTRACE

# the INNER files are lower-level routines
INNER = $(LMd)/alloc_graph.o $(LMd)/create.o $(LMd)/detect.o $(LMd)/init_graph.o

INNERC = $(LMd)/alloc_graph.c  $(LMd)/create.c $(LMd)/detect.c $(LMd)/init_graph.c

# the OUTER files are seen by the outside world
OUTER = $(LMd)/detect.o

OUTERC = $(LMd)/detect.c

DLockDetect.o : $(INNER) # call our module dlockdetect.o
	ld -r  $(INNER) -o DLockDetect.o

$(INNER): $(WISS)/sm.h $(WISS)/wiss.h $(WISS)/page.h $(LM)/lockquiz.h \
	$(LM)/LM_macro.h  $(WISS)/locktype.h

EXECS:
	echo $(INNERC) $(OUTERC) >> ../EXECS

lint : $(INNERC) $(OUTERC)
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) $(INNERC) $(OUTERC) >lint.out

ens:
	enscript -2r -G -P7a $(INNERC)

clean :
	/bin/rm -f *.o
