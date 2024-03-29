# Makefile for level 3 of WiSS

INCLUDE = ..
LOCK = ../LM

CFLAGS = -I$(INCLUDE) -I$(LOCK) $(DEBUGFLAG) $(TRACEFLAG) $(PROF)

# the INNER files are lower-level routines
INNER = AM_error.o  AM_boolean.o AM_scantable.o AM_commit.o AM_commit1.o

INNERC = AM_error.c  AM_boolean.c AM_scantable.c AM_commit.c AM_commit1.c

# the OUTER file is level 3 interface routines, seen by level 4 ? 

OUTER =	am_setcursor.o am_setscan.o    am_init.o       am_final.o\
	am_openfile.o   am_closefile.o am_openindex.o  \
	am_filescan.o  am_longscan.o   am_indexscan.o  am_closescan.o\
	am_fetch1st.o  am_fetchlast.o  am_fetchnext.o  am_fetchprev.o\
	am_insertl.o   am_insertscn.o  am_deletel.o    am_delscan.o\
	am_updscan.o   am_updlong.o    am_readlong.o   am_newindexscan.o\
	am_readscan.o   am_log.o \
	am_geticursor.o  am_setiscan.o   am_openhash.o  am_hashscan.o \
	am_filelocking.o am_createvol.o am_destroyvol.o

OUTERC = am_setcursor.c am_setscan.c    am_init.c       am_final.c\
	 am_openfile.c   am_closefile.c am_openindex.c  \
	 am_filescan.c  am_longscan.c   am_indexscan.c  am_closescan.c\
	 am_fetch1st.c  am_fetchlast.c  am_fetchnext.c  am_fetchprev.c\
	 am_insertl.c   am_insertscn.c  am_deletel.c    am_newindexscan.c\
	 am_delscan.c   am_updscan.c   am_updlong.c     am_readlong.c\
	 am_readscan.c   am_log.c    \
	 am_geticursor.c  am_setiscan.c	am_openhash.c  am_hashscan.c \
	 am_filelocking.c am_createvol.c am_destroyvol.c

# Here come the dependencies
# first, the thing which holds it all together
wiss3.o : $(INNER) $(OUTER) 
	@echo ld -r ~~~ -o wiss3.o
	@ld -r $(INNER) $(OUTER)  -o wiss3.o


# all the object files depend on some header files
$(OUTER) $(INNER) : $(INCLUDE)/wiss.h $(INCLUDE)/page.h \
	   $(INCLUDE)/am.h $(INCLUDE)/am_error.h 

AM_error.o : am_error.i

am_error.i : am_error.m4
	m4 $(INCLUDE)/handler.m4 am_error.m4 > am_error.i

$(INCLUDE)/am_error.h : am_error.m4
	m4 $(INCLUDE)/defs.m4 am_error.m4 >$(INCLUDE)/am_error.h

tags : $(INNERC) $(OUTERC)
	@ctags $(INNERC) $(OUTERC) 

lint : $(INNERC) $(OUTERC)
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) -I$(LOCK) $(INNERC) $(OUTERC) >lint.out
local : $(OUTER) $(INNER)

clean :
	/bin/rm -f *.o
