.KEEP_STATE:

WISS=  /home/lkh/intelssd/wiss/wiss

#DEBUGFLAG	= -DDEBUG
#TRACEFLAG	= -DTRACE
#PROF		= -g
#PROF		= -O
#EVENTFLAG	= -DEVENTS
#PIC		= -PIC
SYNCHDISK	= -D_SYNCHDISK

# Compiler and partial loader for UNIX 4.0 to use
CC = gcc
LOADER1=        ld -Bstatic -r

CFLAGS = -I$(WISS) $(DEBUGFLAG) $(TRACEFLAG) $(PROF) $(PIC)

OPTIONS = DEBUGFLAG='$(DEBUGFLAG)' TRACEFLAG='$(TRACEFLAG)' PROF='$(PROF)'\
	EVENTFLAG='$(EVENTFLAG)' CC='$(CC)' TARGET_ARCH='$(TARGET_ARCH)' \
	WISS='$(WISS)' PIC='$(PIC)'

# sub-directories at this level
SUBDIR = 0 1 2 3 util misc LM SM

# sub-modules created in the sub-directories

SUBOBJS	= 0/wiss0.o 1/wiss1.o 2/wiss2.o 3/wiss3.o \
	  util/util.o LM/lm.o SM/sm.o

wiss.o : $(SUBOBJS)
	ld -r $(SUBOBJS) -o wiss.o

MAKE	= make

0/wiss0.o : do_it_always
	@echo ------------------------------------------------
	(cd 0; $(MAKE) $(OPTIONS) SYNCHDISK='$(SYNCHDISK)')
1/wiss1.o : do_it_always
	@echo ------------------------------------------------
	(cd 1; $(MAKE) $(OPTIONS))
2/wiss2.o : do_it_always
	@echo ------------------------------------------------
	(cd 2; $(MAKE) $(OPTIONS))
3/wiss3.o : do_it_always
	@echo ------------------------------------------------
	(cd 3; $(MAKE) $(OPTIONS))
util/util.o : do_it_always
	@echo ------------------------------------------------
	(cd util; $(MAKE) $(OPTIONS))
LM/lm.o : do_it_always
	@echo ------------------------------------------------
	(cd LM; $(MAKE) $(OPTIONS))
SM/sm.o : do_it_always
	@echo ------------------------------------------------
	(cd SM; $(MAKE) $(OPTIONS))

do_it_always:

lint :  do_it_always
	lint -I. -I./LM/. 0/*.c 1/*.c 2/*.c 2/record/*.c 2/btree/*.c \
	2/hash/*.c 2/directory/*.c \
	3/*.c util/*.c misc/*.c LM/lockmanager/*.c LM/deadlock/*.c SM/*.c 

clean:
	for i in ${SUBDIR}; do (cd $$i; $(MAKE) clean); done;
	/bin/rm -f  *~ TAGS execs cflow .make.state*

# The files headers and execs contain the abolute pathnames of all
# wiss files. They can be used by ctags and cflow.

# NOTE : Whenever source files are deleted or created, execs or
# headers should accordingly be deleted to allow the update of
# tags and cflow.

execs :	
	rm -f execs
	touch execs
	for i in $(SUBDIR); do (cd $$i; $(MAKE) WISS='$(WISS)' execs); done

TAGS:	execs 
	etags `cat execs`

cflow:	execs 
	cflow -I$(WISS) -r `cat execs` > cflow

