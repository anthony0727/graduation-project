
.KEEP_STATE:

PROF=-g 
#PROF=-O -pg
#PROF = -O
TRACEFLAG =
DEBUGFLAG =  
# to run the hierarchy locking in lock_page, remove the #
#HIER= -DHIERARCH_LOCK

# current directory
LMs= 	.
WISS=	..
LOCK=   .

CFLAGS = -I$(LMs) -I$(WISS) -I$(LOCK) $(DEBUGFLAG) $(TRACEFLAG) $(PROF) $(HIER)

# compiler options to be passed dowm
OPTIONS = TRACEFLAG='$(TRACEFLAG)' DEBUGFLAG='$(DEBUGFLAG)'\
	PROF='$(PROF)' LM='../$(LMs)' HIER='$(HIER)'

LOCKMANAGER =	lockmanager/LockManager.o
DEADLOCK = 	deadlock/DLockDetect.o

LINTFILES = deadlock/alloc_graph.c deadlock/create.c deadlock/detect.c\
	deadlock/init_graph.c lockmanager/LM_event.c lockmanager/alloclock.c\
	lockmanager/create.c lockmanager/initlock.c lockmanager/locker.c\
	lockmanager/printstat.c lockmanager/release.c 

# sub-directories
SUBDIR = lockmanager deadlock

lm.o : $(LOCKMANAGER) $(DEADLOCK) $(LOCK)/lockquiz.h
	ld -r $(LOCKMANAGER) $(DEADLOCK) $(INNER) -lm -o lm.o

$(LOCKMANAGER): do_it_always
	(cd lockmanager; make $(OPTIONS))

$(DEADLOCK): do_it_always
	(cd deadlock; make $(OPTIONS))

do_it_always:

lint : 
	lint -DUSEPROC -I$(LMs) -I$(LOCK) -I$(WISS) $(LINTFILES) >lint.out

EXECS :
	echo $(INNERC) > EXECS
	for i in $(SUBDIR); do (cd $$i; $(MAKE) LM='.' EXECS); done
	chmod 664 EXECS

TAGS:	EXECS
	etags `cat EXECS`
	chmod 664 TAGS

CFLOW:  EXECS
	cflow -I$(LMs) -I$(WISS) -I$(LOCK) `cat EXECS` > CFLOW

CFLOW_R:  EXECS
	cflow -I$(LMs) -I$(WISS) -I$(LOCK) -r `cat EXECS` > CFLOW_R

clean :
	/bin/rm -f *.o lm EXECS CFLOW TAGS
	for i in ${SUBDIR} test; do (cd $$i; echo `pwd`; make clean); done;
