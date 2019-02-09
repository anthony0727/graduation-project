# Makefile for level 2 of WiSS

INCLUDE = ..
LOCK = ../../lm
RECORD = record
BTREE = btree
HASH = hash
DIRECTORY = directory


# sub-directories
SUBDIR = $(RECORD) $(BTREE) $(HASH) $(DIRECTORY) 

TRACEFLAG = -DTRACE
CFLAGS = -I$(INCLUDE) -I$(LOCK) $(DEBUGFLAG) $(TRACEFLAG) $(PROF)

# the OUTER file is level 2 initialization routine

# compiler options to be passed dowm
OPTIONS = TRACEFLAG='$(TRACEFLAG)' DEBUGFLAG='$(DEBUGFLAG)' PROF='$(PROF)'

install : subdirs 
	ld -r $(RECORD)/wiss2r.o $(DIRECTORY)/wiss2d.o $(BTREE)/wiss2b.o \
	      $(HASH)/wiss2h.o -o wiss2.o

subdirs:
	for i in ${SUBDIR}; do (cd $$i; make $(OPTIONS) ); done;

$(RECORD)/r_error.i : $(RECORD)/r_error.m4
	cd $(RECORD); make r_error.i
$(BTREE)/bt_error.i : $(BTREE)/bt_error.m4
	cd $(BTREE); make bt_error.i
$(DIRECTORY)/d_error.i : $(DIRECTORY)/d_error.m4
	cd $(DIRECTORY); make d_error.i

lint : $(INNERC) $(OUTERC)
	@echo lint ~~~ '>lint.out'
	@lint -I$(INCLUDE) $(INNERC) $(OUTERC) >lint.out

load :
	@ld -r $(RECORD)/wiss2r.o $(DIRECTORY)/wiss2d.o $(BTREE)/wiss2b.o \
	$(HASH)/wiss2h.o $(UNIX)/wiss2u.o $(OUTER) $(INNER) -o wiss2.o
	@chmod -x wiss2.o

clean :
	/bin/rm -f *.o
	for i in ${SUBDIR}; do (cd $$i; make clean); done;
