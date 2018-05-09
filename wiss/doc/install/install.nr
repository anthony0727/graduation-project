.ll 6.5i
.ls 1
.he ''%''
.po 1.0i
.ce 20
\fBInstalling, Recompiling, and Maintaining 
the System V Multiuser Version of WiSS\fR
.sp
David J. DeWitt
Computer Sciences Department
University of Wisconsin
Madison, WI 57306
.sp
November 1990
.ce 0
.fi

.sh 1 "Introduction"
.pp
In this document,  how to install and recompile the System V
version of the Wisconsin Storage System (WiSS).
Section 2 describes how to extract files from the distribution
tape.  The initial process of configuring and compiling
the system is presented in Section 3.
Section 4 contains a brief overview of how to get started
using WiSS.  
Section 5 describes how to format a WiSS volume, running WiSS either on
top of the UNIX file system or on raw devices.
A description of the directory structure of WiSS is given in Section 6.
A more detailed description of system recompilation
can be found in Section 7.
.br
.sh 1 "Installation"
.pp
The first step in installing the system is to extract the source
code from the tape.  The tape was written using tar with a block
size of 20.  Extracting
the tape will produce four directories:  bin, doc, test, and wiss.
After you recompile the system the bin directory will contain 
a file called \fIformat\fR which is used to format WiSS volumes
and the library \fIwiss.o\fR which is the compiled WiSS system.
The doc directory holds the documentation.  The test directory
contains a set of test programs used to test the system,
and the wiss directory contains the actual source code.
If you don't like this directory structure feel free to change
it.  Doing so, however, will require that you change the make files
in the test subdirectories and the wiss directory.
.pp
To extract the tape type:  
.sp
.ti 1.0i
tar xvbf 20 tapedrivename
.sp
Where \fItapedrivename\fR is the logical name of your tape drive.
.br
.sh 1 "Configuring, and Compiling the System"
.sh 2 "Unix System V"
.pp
You should begin the installation process by checking whether
your kernel has the System V shared memory primitives compiled
into it.  The release of Sun 4.1 that we received had these mechanisms
disabled.  Ultrix has them installed but the default configuration
is useless.  In recompiling your kernel you need to address
two issues.  First, the system needs to be configured with
as many semaphores as you expect concurrent processes.
Thus, if you are going to support a maximum of 16 users the system needs
to be configured with 16 semaphores.  
Since the rest of the software comes configured for 32 users,
we suggest that you set this value to 32.
Second, the size of the maximum shared memory segment and the amount of
shared memory available must to be configured properly.
If you are going to use a 4 megabyte WiSS buffer pool,
the kernel should be configured so that the maximum
shared memory segment size is at least 8 least megabytes
and that the amount of shared memory space available 
is at least 8 megabytes.  A good rule of thumb is to
insure that both these constants are at least twice
the size of the buffer pool that you plan on using.
.br
.sh 2 "Configuring the System"
.pp
Before compiling the system, you may (but do not need to) 
want to adjust several constants.
Begin by changing to the wiss directory.  There you will 
find four key files:  wiss.h, sm.h, latch.h, lockdefs.h.
At the beginning of the file wiss.h you will find 
a set of constants:  PAGESIZE, the size of a WiSS data page, 
MAXBUFS, the number of frames in in the buffer pool,  
MAXVOLS, the maximum number of simultaneously
mounted disk volumes, and MAXOPENFILES, the maximum number of
files that can be opened simultaneously.  Our experience
is that a page size of 4K or 8K is optimal for most applications
(Gamma uses 8K byte pages and O2 uses 4K byte pages).
The system comes configured with a 4K byte page size.
MAXBUFS is the parameter that you most likely will want to change.
As distributed, MAXBUFS is set to 1024, giving a 4 megabyte
buffer pool.  If you change this value make sure
that the operating system is configured to allow a shared memory
segment approximately twice this size.
You will probably find the settings for default
settings for MAXOPENFILES and MAXVOLS acceptable.
.pp
In sm.h the only value that you might want to change is LOCKTABLESIZE.
LOCKTABLESIZE is the size of the hash table used  
used by the buffer pool code for setting short term 
page latches.  Since these latches are held for a very 
short period of time (basically for the duration of one 
buffer pool operation), a small hash table is sufficient.
The second value you might find a need to change is
.pp
In latch.h, the maximum number of concurrent users is set
via the constant MAXPROCS.  The default value is set at 32
but can be changed to any value desired (again keep in mind
that there must be one System V semaphore for each active process).
Two structures depend on this value.  First, in the file sm.h, 
the structure SMDESC (which is the key structure controlling shared 
memory) is an array named users of size MAXPROCS.
The other place where MAXPROCS is used is in type declaration
LATCH in the file latch.h.  On architectures without a test 
and set instruction (e.g. Mips 2000 and 3000),  a variant of Dekkers algorithm
is used for process synchronization (more on how synchronization
works is described in Section 2 of the document \fIArchitectural
Overview of Multiuser WiSS under System V shared memory\fR).
This algorithm requires a flag byte for each active process and hence each
latch for such architectures needs to contain an array of MAXPROCS chars.
(On architectures with a test and set instruction, the make files
automatically remove this array).
.pp
Finally, in lockdefs.h the size of the hash table used for setting
locks is specified by the constant MAXRES.  The default value
for this constant is 32768.  This is not a limit on the
maximum number of locks that can be held concurrently.  It simply
defines the hash table size (chained bucket hashing is used).
However, if the number of concurrent locks begins to approach
this number you might want to raise it.  If you change it make sure
that you set the constant MAXRES_1 to MAXRES minus 1.  
.br
.sh 2 "Recompilation"
.pp
To recompile the system simply type "make" in the wiss directory.
The make file will invoke the lower level make files automatically.
Making the system produces two files:  format and wiss.o,  both of which 
get automatically moved to the bin directory 
(which is at the same level as the wiss directory itself).
The make files automatically determine the target architecture
so that the same source code can compiled on a mips machine,
a sun4, a 680XX, or a VAX without any modifications.  The only
place where assembly language is used is in the file testandset.S
in the SM subdirectory.  The make file in this directory
automatically takes care of assembling the correct assembly
language for obtaining access to the "test and set" instruction
provided by the hardware (support for sparc, vax, 680XX, and 386 is
included).
.pp
As distributed, the system is compiled using the -g flag.  This
can be changed by simply changing the top level make file.
Comment out the line "PROF = -g" and uncomment out the line "PROF = -O"
in the top level make file only.
.pp
We have also encountered some problems with loader that comes
distributed with the Sun 4.1 operating system (basically it is buggy).
In several directories (but not all) you will find three make files.  The ones
with the .normal extension are the ones that we normally use under Ultrix. 
The ones with the .sun4.1 extension include work arounds to get around
the loader bugs.  As distributed, the make file
that actually gets invoked is the one that will work correctly
on the Sun 4.1 operating system.   Thus, if you are building
the system for this version of Sun OS you should not have to do
anything (this same make file actually will work fine under Ultrix
also and Sun 4.0 too).
.br
.sh 1 "Getting Started"
.pp
Probably the easiest way to get started is by looking at an
example.  Change to the test directory.  There you will
find a number of subdirectories:  acobbenchmark, hashtests,
locktests, setbenchmarks, sunbench.btree, and sunbench.hash.
The acobbenchmark contains the benchmark used to evaluate
workstation server architectures (see the VLDB 90 paper on
alternative workstation/server architectures for more details
on this benchmark).
The hashtests directory contains a series of stress tests
for WiSS's extensible hashing access method.  The locktests
directory contains some code for testing locks.  In the
setbenchmark directory you will find a number of programs
that implement various forms of sets.  The two directories
sunbench.btree and sunbench.hash contain implementation
of the Sun benchmark the first using a b-tree and the second
using hashing.  Each directory has a file called RunTests
which will compile each test program and run them all.
I suggest that you start in the setbenchmarks directory
with the programs loaddb.c and scan.c.  loaddb.c loads
a standard Wisconsin benchmark relation and scan.c scans it.
To run this pair of programs begin by typing "make loaddb scan".
Doing so will compile the corresponding .c's and link 
the resulting .o's with wiss.o to produce an executable
for each.
Before running loaddb,  you need to make a WiSS "disk". 
To do this type "make disk". The make file will invoke the "format"
utility in the bin directory to format a disk according to
the disk specification in disk.script.  Basically, WiSS was
designed to run on raw disk drives (this is, for example,
how WiSS is used on Gamma) and "format" formats a disk
volume for use.  (NOTE!!! On our machines there is a file in 
/usr/misc/format which is not to be confused with the WiSS "format"
command).   When format is run on top of the unix file
system it produces a file corresponding to the disk volume.
The name and characteristics of that file are determined
from the script in disk.script.  For more on formatting
volumes see Section 5 below.
.pp
One you have compiled loaddb and scan and made a WiSS "disk"
you are ready to run.  Typing "loaddb foo 10000 TRUE" will load
a 10,000 tuple relation named foo.  
Typing "scan foo 10001 0 TRUE" will scan foo.  A simple
demonstration of the multiuser capability is to load relations foo
and bar simultaneously from two different windows.  Or load
foo once and then scan it simultaneously from two different windows.
.pp
Sometimes when user programs crash they will leave terminate
without cleaning up the shared memory segment (a task which 
is done only if an exiting process finds
it is the last process attached to the shared memory segment)
or removing its semaphore.
In the case of the shared memory segment, the next process
that runs will take care of cleaning up the old segment
(in the future you may want to put some recovery logic here).
However, the semaphore must be removed manually.
To do this, the Unix command "ipcs" will show all
active shared memory segments and semaphores and the
the command "ipcrm -s N" will remove semaphore number N.
The command "ipcrm -m M" will remove shared memory segment M.
.br
.sh 2 "Interfacing with WiSS"
.pp
WiSS is intended to be used as a set of library routines,
that provide a storage sub-system to higher level users.
To interface with WiSS, a user program needs to include
three header files, \fIwiss.h\fR, \fIwiss_r.h\fR (from \fIwiss\fR),
and \fIlockquiz.h\fR (from \fIwiss/LM\fR).
The first file, \fIwiss.h\fR, contains all the data structure definitions
that a WiSS user needs.
The second file, \fIwiss_r.h\fR, maps official WiSS routine names
into their real names in the system.
The third file, \fIlockquiz.h\fR defines lock modes (e.g. l_X, l_S, ...)
and lock manager error codes.
After a user program is compiled, the final step is to
link the user's object code with \fIbin/wiss.o\fR, which contains the
entire object of WiSS.
.sh 1 "Initializing Storage Devices for WiSS"
.pp
The storage model of WiSS has volumes (disk packs, tapes, etc.) 
mounted on devices (disk drives, tape drives, etc.).
Running on top of the UNIX file system, a volume is assigned to
a single UNIX file. 
.ft B
The name of the UNIX file is also the name of the virtual device the volume is
attached to.
.ft R
How to run WiSS on top of raw devices will be discussed later in this section.
.pp
A WiSS volume needs to be initialized before users can actually 
stored data on it.
The initialization required for a WiSS volume includes: 
labeling the volume with a symbolic name; 
assigning an integer which uniquely identifies the volume; 
recording a description of the physical attributes of the volume.
Two pieces of information are needed to describe the physical layout
of a volume under WiSS, the number of pages in an extent and the total 
number of extents on the volume.
(A cylinder on a disk, for example, may be used as an extent.)
.pp
There is a volume formatter for WiSS in \fIwiss/misc/format.c\fR.
This program formats a volume according to WiSS's conventions.
Four pieces of information, as described above, are required by the formatter:
.(l
Volume Label : an ASCII name describing the volume
Volume ID : an integer that uniquely identifies a volume
Number of Extents : an positive integer 
Number of Pages per Extent : an integer larger than 3
.)l
The formatter can be activated either interactively or through
a script file which contains the above information, 
.ul
each on a separate line.
.pp
For example, to "create" and format a volume mounted on the device 
named "wiss_disk", we can prepare a 4-line script file :
.(l
Volume foo
1
20
40
.)l
This script file tells the formatter that the volume contains 20
extents, each of which has 40 pages. 
The volume created will have the label "Volume foo" and an Volume ID of 1.
Let the script file be "disk.script", the
following UNIX command will create a volume as described.
.sp
.ce
\fI format - wiss_disk < disk.script \fR
.sp
.pp
Formally, the command syntax for evoking the formatter is:
.sp
.ce
\fI format [-] [-i] devicename \fR
.sp
The -i flag implies an already-existing volume, and just prints out the 
volume information.
The flag - is for reading specifications from a redirected input; 
parameters read are echoed. 
.br
.sh 2 "Initializing a raw device"
.pp
Under UNIX, a raw device is treated as a "character special file".
There is a 'raw' interface on UNIX which provides for direct
transmission of blocks between the disk and the user's buffer.
The block I/O's of a "raw file" bypass the physical address translation and
block buffering of the file system,
and thus avoid the overhead of the UNIX file system. 
To deal with a 'raw device', we treat it as 
a UNIX file, which has a system-defined name such as "/dev/rmt8".
The system calls (read, write, seek, etc.) are analogous to those
for conventional files,
with some restrictions on the number of bytes that can be transferred.
One important thing to remember here is that one must be careful in
setting up the extent structure so that it matches the physical 
properties of the real device.
For example, a cylinder is a good candidate for an extent if files
are expected to be large.
.br
.sh 1 "Directory Structure (Road Map)"
.pp
The WiSS home directory is organized into the following sub-directories:
\fIwiss, doc\fR, and \fItest\fR.
The source code for WiSS is stored in \fIwiss\fR, which in term is organized
into several sub-directories that correspond to each individual level of WiSS
and two libraries.
The \fIdoc\fR directory contains all the on-line documents 
(in ditroff format) for WiSS,
including a design specifications, an implementation note, a user manual,
and this document itself.
There are a few test programs in \fItest\fR for testing WiSS and
for demonstrating the procedural protocol for using WiSS as a set of library
routines.
.pp
The source (\fIwiss\fR) directory contains all the header include files,
which are either private to a particular level or global to all levels of WiSS.
The actual source code is distributed among the following sub-directories:
.np
0 : level 0 (input/output level) source code.
.sp
The routines in this directory serve two purposes, 
storage allocation and physical device operations.
To support these functions, the routines maintains and/or
access a volume table that describes all the mounted (on-line) volumes.
The volume headers of all the mounted volumes are cached by this
table to provide the necessary information for allocating storage
space on the volumes.
.np 
1 : level 1 (buffer management level) source code.
.sp
This level manages a pool of page buffers and implements
a page replacement algorithm.
The role of the routines in this directory is 
to maintain a buffer table that keeps track of the status of
all the buffers in the buffer pool and to manage paging activities effectively.
.np
2 : level 2 (storage structure level) source code.
.sp
Due to the complexity of storage structures, level 2 is
further divided into several modules: 
\fIrecord, btree, hash, and directory\fR.
The routines in \fIrecord\fR implement sequential files and provide
a record-at-a-time interface for accessing, inserting and deleting records.
In addition, long data items are implemented here as a special type
of fields.
.sp
The B+tree structures are implemented by the routines in \fIbtree\fR.
These routines supports not only standard B+tree algorithms (traversal,
insertion and deletion),
but also an efficient mechanism for building a B-tree from bottom up
(in batch).
An external merge sort utility is included here for building B+trees.
However, this sort utility is also available to higher level users.
The hash structures implement an access method based on extendible
hashing and are implemented by the routines in \fIhash\fR plus
some of the B+tree and record routines.
.sp
Level 2 supports a flat symbolic file directory.
The translation of symbolic names into internal file identifiers, 
as well as the maintenance of other file information, are
done by the routines in \fIdirectory\fR through the use of 
.ul
file descriptors 
stored in file directories.
An in-core open file table is maintained here to
speed up the access to the descriptors of active files.
.np
3 : level 3 (access method level) source code.
.sp
This level supports the concepts of scan cursors and search predicates.
A scan table is maintained by the routines at this level to keep track
of all the scans in progress.
.np
util : library routines used by most levels of WiSS.
.sp
This directory provides library routines for the rest of WiSS.
.np
misc : the volume formatter.
.sp
The volume formatter is a separate program that must be used
before a volume is usable by WiSS.
.br
.sh 1 "System Generation"
.pp
Most of the WiSS routines are written in C.
To make \fI#include\fR statements easier to manage,
each routine assumes all the header files are in a
standard \fIinclude\fR directory and uses statements
like \fI#include <foo.h>\fR.
However, since the WiSS header files are not really in a
standard \fIinclude\fR directory that the C compiler knows about,
the compiler option \fI-I\fR must be used to re-direct the compiler
to look for header files in \fIwiss\fR.
.pp
Error codes are used extensively by WiSS routines.
Each level of WiSS defines its own set of error codes
with one restriction : the second least significant digit (decimal)
must equal to the level number \**.
.(f
\**
This makes the identification of an error code easier.
.)f
Since error codes are locally defined, 
an error interpreter (error message reporter) is also provided by each level.
Maintaining the error codes and the error interpreters can be a painful task.
Therefore, the \fBm4\fR macro processor is used to automate the process.
To define an error code for a local module, an entry is added to a local
file with a \fI.m4\fR suffix. 
The \fBm4\fR macro processor is then evoked to update two files:
a header file in \fIwiss\fR which defines all the error codes for the module;
a local file which consists of \fIprintf\fR's and is included by
the local error interpreter.
The \fBm4\fR macros for generating the above two files are defined
in \fIdefs.m4\fR and \fIhandler.m4\fR in the directory \fIwiss\fR.
.pp
It may seem complicated to generate the object code for WiSS.
However, a hierarchy of \fImake\fR files has been embedded in the source
directory to do the dirty work.
There is a "Makefile" in \fIwiss\fR that can re-generate the entire
WiSS object.
It will, in term, evoke the appropriate make files when necessary.
Therefore, 
.ft B
the task of system generation is simply to do
a \fImake\f in the source directory \fIwiss\fR.
.ft R
This will produce an object file called \fIwiss.o\fR in \fIwiss\fR
that contains all the routines in WiSS.
.pp
There are a TRACE flag and a DEBUG flag in each of those make files.
However, the use of those flags is intended for debugging WiSS only.
Running WiSS under the DEBUG and/or TRACE mode may result 
in performance degradation and unexpected trace messages to appear
on the standard output device.
An application program should be linked to an object of WiSS compiled with
both flags \fBoff\fR.
The following sub-sections are included just in case there is a need to
probe into the system, say, when a bug of WiSS is suspected.

.sh 2 "Tracing WiSS Execution"
.pp
There are two levels to the tracing facilities in WiSS: compile time
and run time.
A decision is made at compile time whether the routine(s) in a source
file will be traceable or not.
If it is decided to enable tracing, then trace output will be available
depending on run-time flags supplied.
If COMPILE-time trace is not enabled, then trace output will not be available,
regardless of the run-time flags given.
.pp
Every WiSS routine has code immediately after its variable
declarations which is capable of printing
.(l
- the name of the routine
- the names and values of each of the parameters
.)l
Parameters which are pointers are dereferenced, and the actual
values pointed to are printed out, unless it is the address which
is important to the routine, and not the data being pointed to
(examples are parameters through which values are returned, and very
very large data structures such as 4K-byte pages).
The compile-time check for trace enabling is the existence of a
macro variable TRACE.
The C macro statement
.(l
#ifdef TRACE
.)l
will test for the existence of this variable.
To enable tracing, invoke the C compiler with an argument
"-DTRACE" which will define the variable TRACE.

.pp
There are 5 trace variables to control run-time tracing: one for each level,
named Trace0, Trace1, Trace2, Trace3; and one for utility routines which
are not level-specific, Traceu.
A declaration of the form
.(l
extern TRACEFLAGS       Trace1;
.)l
provides proper access to the Trace variables.
Tracing is by subsystem: according to the run-time flags, a bit is set
for each sub-system whose trace is enabled.
For each sub-system, #define a variable of the form "tSUBSYSTEMNAME".
The values start at 0 and increase by 1.
Examples of sub-systems for level 0 are bit manipulation routines,
interface routines for higher levels, and physical I/O.

.pp
The run-time check for trace enabling is a C "if" statement inside
the #ifdef/#endif lines.
A sample test in level 3 for whether or not a sub-system is enabled is
.(l
if (checkset(&Trace3, tTHISSUBSYSTEM))
{
    printf("thisroutine (parm1=%d, parm2=0x%x)\\n", parm1, parm2);
}
.)l
In this example, there are two parameters; one is an integer, the
other is a pointer.
The routine "checkset" is a bit-map routine which checks whether
a particular bit within a bit-map is set.

.pp
The routine to read the command line flags and set the appropriate
variables is in "wiss/util/traceflags.o".
The routine will typically be called from the main program:
.(l
main(argc,argv)
int	argc;
char	**argv;
{
        wiss_checkflags (&argc, &argv);
        ...
}
.)l
The routine will scan the command line for WiSS trace flags, set
the trace variables, and remove the trace flag arguments from the
command line.
This modifies argc and argv--note the ampersands preceding them in
the call.
.pp
The command line flags are of the form
.(l
-Tlsss...
.)l
Where "l" is the level number (0, 1, 2, 3, or u for utilities),
and "s" (one or more) is the number of the sub-system to enable.  
This number starts at "0" (corresponding to tSOMETHING = 0), goes through
"9", "a" and up to "v" (corresponding to tSOMETHING = 31).
This number can also be "+", meaning "enable all sub-systems at this level".
Some examples:

.nf
.in 4
.ti 0
-T0012
.br
enable sub-systems 0,1 and 2 at level 0
.sp
.ti 0
-T0+ -T10
.br
enable all of level 0, and sub-system 0 of level 1
.sp
.ti 0
-T0135 -T13 -T02
.br
enable level 0 sub-systems 1,2,3,5; and level 1 sub-system 3
.in 0
.fi

.sh 2 "Debugging Facilities in WiSS"
.pp
The debugging facilities in WiSS are similar to the tracing
facilities described above.
However, debugging statements appear only at places that are thought
to be critical.
In addition, some parts of WiSS, especially level 2,
will modify (conceptually) certain system parameters under the DEBUG mode.
For example, under the DEBUG mode the B+tree routines will 
shrink the size of a page to force a tree to grow higher
in order to exercise some routines that may otherwise hard to reach.
Therefore, there may be severe performance penalty to run WiSS under this mode.
.pp
Beside the in-line debug statements, there are also some tools available
for probing underneath the system interface that allows one to
either examine the system states or to perform some consistency checking.
Some of the most useful tools \** (routines) are:
.(f
\**
An application program should not depend on the existence
of these routines.
.)f
.np
IO_checker(VolID): level 0 volume header consistency checker.
.sp
This routine performs a consistency check on the volume header of
a volume and reports any inconsistency or error condition.
.np
BF_dumpbuftable(): level 1 buffer table displayer.
.sp 
This routine dumps the entire buffer table in a readable form to the
standard output device.
The status of each page buffer in the buffer pool is displayed;
including its page identifier, file identifier, 
time stamp of the last reference, etc..
The buffer usage information for each user is also displayed.
.np
r_dumplong(FileNum, RecID) RID *RecID: 
level 2 long data item displayer.
.sp
This routine prints the contents of a long data item, assuming that the data
are in ASCII form.
.np
r_dumpfile(FileNum):
level 2 file displayer.
.sp
This routine prints the contents of a file, assuming that the data
are in ASCII form.
Records in the file are printed in their physical order.
.np
bt_print_btfile(FileNum, Root) PID *Root:
level 2 B+tree displayer.
.sp
This routine prints the entire structure of a B+tree from
the root to the leaves one level at a time.
Pages at the same level are printed from left to right.
If Root is NULL, the entire tree is printed.
.np
d_dumptable():
level 2 active file table displayer.
.sp
The routine dumps the open file table maintained by level 2.
Information on the active files is displayed one file at a time.
This includes the run time information (file access mode, user, etc.) as well
as the static information (owner, file identifier, access permission, etc.)
of a currently active file.
.np
AM_dumpscantable():
level 3 scan table displayer.
.sp
This routine dumps the active scan table maintained by level 3.
The information on each scan in progress is displayed one at a time.
This includes the scan identifier, the scan type, 
the open file number of the file being scanned, etc..
.br
