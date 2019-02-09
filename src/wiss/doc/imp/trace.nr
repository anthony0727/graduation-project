.sh 1 "Tracing WiSS Execution"
.(x
.sh 1 "Tracing WiSS Execution"
.)x

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
Parameters which are pointers should be dereferenced, and the actual
values pointed to should be printed out, unless it is the address which
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
.(l
-DTRACE
.)l
which will define the variable TRACE.

.pp
There are 5 trace variables to control run-time tracing: one for each level,
named Trace0, Trace1, Trace2, Trace3; and one for utility routines which
are not level-specific, Traceu.
A declaration of the form
.(l
extern TRACEFLAGS       Trace1;
.)l
provides proper access to the Trace variables.

.pp
Tracing is by subsytem: according to the run-time flags, a bit is set
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
variables is in "~wiss/src/util/traceflags.o".
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
This number starts at "0" (corresponding to tSOMETHING = 0), and
goes up to "v" (corresponding to tSOMETHING = 31).
This number can also be "+", meaning "enable all sub-systems at
this level".
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

