# Makefile for level 2, records part, of WiSS

INCLUDE = ../..
LOCK = ../../LM

CFLAGS = -I$(INCLUDE) -I$(LOCK) $(DEBUGFLAG) $(TRACEFLAG) $(PROF)

# the INNER files are lower-level routines
INNER = r_slide.o	r_dumppage.o    r_dumplong.o\
	r_hookup.o 	r_dropout.o 	r_addrec.o 	r_getrecord.o\
	r_initslice.o   r_crumb.o r_dumpfile.o 

INNERC = r_slide.c 	r_dumppage.c    r_dumplong.c \
	 r_hookup.c 	r_dropout.c	r_addrec.c r_getrecord.c	\
	 r_initslice.c   r_crumb.c r_dumpfile.c

# the OUTER files are seen by the outside world
OUTER = st_appendpage.o st_readrec.o   st_writerec.o \
	st_compare.o   st_insertrec.o st_appendrec.o st_deleterec.o \
	st_firstfile.o st_lastfile.o  st_nextfile.o  st_prevfile.o \
	st_createl.o   st_destroyl.o  st_insertfrm.o st_deletefrm.o\
	st_readfrm.o   st_writefrm.o  st_compressl.o st_ridcomp.o\
	st_readlength.o st_getrec.o st_expandrec.o st_appendfile.o \
	st_scanfile.o

OUTERC = st_appendpage.c st_readrec.c   st_writerec.c \
	 st_compare.c   st_insertrec.c st_appendrec.c st_deleterec.c \
	 st_firstfile.c st_lastfile.c  st_nextfile.c  st_prevfile.c \
	 st_createl.c   st_destroyl.c  st_insertfrm.c st_deletefrm.c\
	 st_readfrm.c   st_writefrm.c  st_compressl.c st_ridcomp.c\
	 st_readlength.c st_getrec.c st_expandrec.c st_appendfile.c \
	 st_scanfile.c

# Here come the dependencies
# first, the thing which depends on everything else
wiss2r.o : $(INNER) $(OUTER) # call our module wiss2r.o
	@echo ld -r ~~~ -o wiss2r.o
	@ld -r $(INNER) $(OUTER) -o wiss2r.o
	chmod -x wiss2r.o
# load it all into a big .o module, make it non-executable (it's incomplete!)

# all the object files depend on some header files
$(INNER) $(OUTER) : $(INCLUDE)/wiss.h $(INCLUDE)/page.h \
		    $(INCLUDE)/st.h $(INCLUDE)/st_r.h \
		    $(INCLUDE)/r_error.h  $(INCLUDE)/sm.h

r_error.i : r_error.m4
	m4 $(INCLUDE)/handler.m4 r_error.m4 > r_error.i

$(INCLUDE)/r_error.h : r_error.m4
	m4 $(INCLUDE)/defs.m4 r_error.m4 >$(INCLUDE)/r_error.h

tags : $(INNERC) $(OUTERC)
	@ctags $(INNERC) $(OUTERC) 

lint : $(INNERC) $(OUTERC)
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) -I$(LOCK) $(INNERC) $(OUTERC) >lint.out

clean :
	/bin/rm -f *.o
