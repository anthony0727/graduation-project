#
#  Makefile for WiSS test programs 
#

# here is the stuff related to WiSS
WISS = /musr/wiss_old/src
#WISSO = $(WISS)/0/wiss0.o $(WISS)/1/wiss1.o $(WISS)/2/wiss2.o $(WISS)/3/wiss3.o\
#	$(WISS)/util/util.o
WISSO = $(WISS)/wiss.o
MISC = $(WISS)/misc
INCLUDE = $(WISS)

# options of the makefile 
.SUFFIXES:	.o .m 
.m.o: 
	@echo compiling $*.m , please wait ...
	vmc -c -I$(INCLUDE) $*.m

# here is the directory related to modula
MCLIB =  /usr/crystal/mc/lib

# here are your own files
EXEC = test1     test2      test3	test4	  test5
OBJS = test1.o   test2.o    test3.o	test4.o	 test5.o
SRCS = test1.m   test2.m    test3.m	test4.m  test5.m
HEADERS = wiss_mc.h wiss_mc.d

$(EXEC): $(WISSO) 
	@echo final loading of $* please wait ...
	@ld $(MCLIB)/vaxustart.o $*.o $(WISSO) $(MCLIB)/vaxulib.a\
	-lc -o $*

test1 : test1.o
test2 : test2.o
test3 : test3.o
test4 : test4.o
test5 : test5.o

#
# "make disk" creates a brand new virtual disk called VIRTUAL_DISK
#
# there should be a file (in this directory) called $(DISK_SCRIPT) 
# which contains a 3 line description of the desired volume.
# in addition, a file called "format.count" is needed for generating
# unique volume IDs

# name and attributes of the virtual device
VIRTUAL_DISK = wiss.disk
DISK_SCRIPT = disk.script

disk:
	rm -f $(VIRTUAL_DISK)
	$(MISC)/format - $(VIRTUAL_DISK) < $(DISK_SCRIPT)

# "make clean" wipes out all the nonvital files - use before logout
clean:
	rm -f core
	rm -f $(OBJS) $(EXEC) $(VIRTUAL_DISK)

