# Makefile for level 2, directory part, of WiSS

INCLUDE = ../..
LOCK = ../../LM

CFLAGS = -I$(INCLUDE) -I$(LOCK) $(DEBUGFLAG) $(TRACEFLAG) $(PROF)

# the INNER files are lower-level routines
INNER =   d_dumptable.o d_directory.o  

INNERC =  d_dumptable.c d_directory.c  

# the OUTER files are seen by the outside world
OUTER = st_mount.o st_dismount.o st_createf.o st_destroyf.o st_rename.o	\
	st_openfile.o st_closefile.o st_mode.o st_chmod.o st_stat.o st_volid.o\
	st_error.o st_init.o st_final.o

OUTERC = st_mount.c st_dismount.c st_createf.c st_destroyf.c st_rename.c \
	st_openfile.c st_closefile.c st_mode.c st_chmod.c st_stat.c st_volid.c\
	 st_error.c st_init.c st_final.c

# Here come the dependencies
# first, the thing which depends on everything else
wiss2d.o : $(INNER) $(OUTER) # call our module wiss2d.o
	@echo ld -r ~~~ -o wiss2d.o
	@ld -r $(INNER) $(OUTER) -o wiss2d.o
	chmod -x wiss2d.o
# load it all into a big .o module, make it non-executable (it's incomplete!)

# all the object files depend on some header files
$(INNER) $(OUTER) : $(INCLUDE)/wiss.h $(INCLUDE)/page.h \
		    $(INCLUDE)/st_d.h $(INCLUDE)/d_error.h $(INCLUDE)/sm.h

d_error.i : d_error.m4
	m4 $(INCLUDE)/handler.m4 d_error.m4 > d_error.i

$(INCLUDE)/d_error.h : d_error.m4
	m4 $(INCLUDE)/defs.m4 d_error.m4 >$(INCLUDE)/d_error.h

tags : $(INNERC) $(OUTERC)
	@ctags $(INNERC) $(OUTERC) 

lint : $(INNERC) $(OUTERC)
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) $(INNERC) $(OUTERC) >lint.out

clean :
	/bin/rm -f *.o
