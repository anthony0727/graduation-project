.KEEP_STATE:

#
# To compile with profile, run:
# make PROF='-O -pg' RUN_TIME='/lib/gcrt0.o' LOADPROF='-p'
#

#PROF=-g -pg
#PROF=-O -pg
PROF = -g
TRACEFLAG =
DEBUGFLAG =  
# gcrt0.o is used if profile. Otherwise, crt0.o is used.
RUN_TIME= /lib/crt0.o
#RUN_TIME= /lib/gcrt0.o
# -pg is used if PROF= ... -pg ...
#LOADPROF= -p

# to run the hierarchy locking in lock_page, remove the #
#HIER= -DHIERARCH_LOCK

LM=		../..
LMs=		$(LM)/server
WISS = 		../../../wiss

CFLAGS = 	-I$(LM) -I$(LMs) -I$(WISS) $(PROF)

# compiler options to be passed dowm
OPTIONS = TRACEFLAG='$(TRACEFLAG)' DEBUGFLAG='$(DEBUGFLAG)'\
	PROF='$(PROF)' LM='../$(LMs)' HIER='$(HIER)'

CC= 		cc
LOADER1=     	ld -Bstatic -e start $(RUN_TIME) $(LOADPROF)
LOADER2=     	-lc

INNER =		test1 test2 test3 test4
INNERC = 	test1.c test2.c test3.c test4.c

test: 		$(INNER)

test1: test1.o $(LMs)/server_lm.o sgtty.o
	$(LOADER1) test1.o $(LMs)/server_lm.o sgtty.o $(LOADER2) -o $@

test2: test2.o $(LMs)/server_lm.o sgtty.o
	$(LOADER1) test2.o $(LMs)/server_lm.o sgtty.o $(LOADER2) -o $@

test3: test3.o $(LMs)/server_lm.o sgtty.o
	$(LOADER1) test3.o $(LMs)/server_lm.o sgtty.o $(LOADER2) -o $@

test4: test4.o $(LMs)/server_lm.o sgtty.o
	$(LOADER1) test4.o $(LMs)/server_lm.o sgtty.o $(LOADER2) -o $@

$(LMs)/server_lm.o: do_it_always
	cd $(LMs); echo --- `pwd` ---; make $(OPTIONS)

do_it_always:

EXECS:
	echo $(INNERC) $(OUTERC) >> ../EXECS

lint : $(INNERC) $(OUTERC)
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) $(INNERC) $(OUTERC) >lint.out

ens:
	enscript -2r -G -P7a $(INNERC)

clean :
	/bin/rm -f *.o $(INNER)

