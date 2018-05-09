# Makefile for level 2, hash file part, of WiSS

INCLUDE = ../..
LOCK = ../../LM

TRACEFLAG = -DTRACE

CFLAGS = -I$(INCLUDE) -I$(LOCK) $(DEBUGFLAG) $(TRACEFLAG) $(PROF)

# the INNER files are lower-level routines
INNER =   h_util.o h_dump.o

INNERC =  h_util.c h_dump.c

# the OUTER files are seen by the outside world
OUTER = st_createhash.o st_destroyhash.o st_openhash.o \
	st_inserthash.o st_deletehash.o st_gethash.o st_nexthash.o

OUTERC = st_createhash.c st_destroyhash.c st_openhash.c \
	st_inserthash.c st_deletehash.c st_gethash.c st_nexthash.c

# Here come the dependencies
# first, the thing which depends on everything else
wiss2h.o : $(INNER) $(OUTER) # call our module wiss2h.o
	@echo ld -r ~~~ -o wiss2h.o
	@ld -r $(INNER) $(OUTER) -o wiss2h.o
	chmod -x wiss2h.o

# all the object files depend on some header files
$(INNER) $(OUTER) : $(INCLUDE)/wiss.h $(INCLUDE)/page.h \
		    $(INCLUDE)/st_h.h $(INCLUDE)/sm.h


tags : $(INNERC) $(OUTERC)
	@ctags $(INNERC) $(OUTERC) 

lint : $(INNERC) $(OUTERC)
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) -I$(LOCK) $(INNERC) $(OUTERC) >lint.out

clean :
	/bin/rm -f *.o
