# Make miscellaneous WiSS programs

#DEBUGFLAG = -DDEBUG
#TRACEFLAG = -DTRACE

INCLUDE = ..
BIN= ../../bin
CFLAGS = -g -I$(INCLUDE) $(DEBUGFLAG) $(TRACEFLAG)
#CFLAGS = -O -I$(INCLUDE) $(DEBUGFLAG) $(TRACEFLAG)
UTIL = $(INCLUDE)/util

format : format.o $(BIN)/libWiSS.a
	@echo gcc format.o ~~~ -o format
	@gcc format.o -L$(BIN) -lWiSS -g -o format

format.o: $(INCLUDE)/wiss.h $(INCLUDE)/sm.h

clean :
	/bin/rm *.o
#
