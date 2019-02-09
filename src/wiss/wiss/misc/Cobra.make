# Make miscellaneous WiSS programs

#DEBUGFLAG = -DDEBUG
#TRACEFLAG = -DTRACE

INCLUDE = ..
CFLAGS = -g -I$(INCLUDE) $(DEBUGFLAG) $(TRACEFLAG)
#CFLAGS = -O -I$(INCLUDE) $(DEBUGFLAG) $(TRACEFLAG)
UTIL = $(INCLUDE)/util

format : format.o ../wiss.o
	@echo cc format.o ~~~ -o format
	@cc format.o ../wiss.o -g -o format
#	mv format $(BIN)

format.o: $(INCLUDE)/wiss.h $(INCLUDE)/sm.h

clean :
	/bin/rm *.o
#
