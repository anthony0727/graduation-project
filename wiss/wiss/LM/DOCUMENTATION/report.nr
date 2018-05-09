.po 1.1i
.fo '''%'
.ce
\fB\s+3A TABLE DRIVEN LOCK MANAGER FOR THE EXODUS ENVIRONMENT\s0\fR
.sp 2
.ce
\fI\s+2V. Srinivasan\s0\fR
.sp 1
.ce
\fIUniversity of Wisconsin\fR
.ce
\fIOctober 8, 1989\fR
.ls 2
.sh 1 "INTRODUCTION"
.pp
The table driven lock manager is based on a modification to the MWISS 
lock manager [GHAN].
Such a lock manager can be flexibly used with systems having different
sets of lock modes and compatibility and convertibility graphs.
The MWISS lock manager is a highly optimized version that uses macros to
enhance performance. The modified lock manager maintains
the same high performance for simple lock calls while introducing
more features like lock upgrades [GRAY], conditional locks and instant 
duration locks [MoLe].
.pp
The following sections briefly describe the tables necessary for the definition
of lock modes and the type of lock calls available. 
This document describes the lock manager interface only.
The internal structure of this lock manager resembles 
almost entirely the MWISS lock manager [GHAN] and hence the details are not 
reproduced here. 
.sh 1 "LOCK MODES and DESCRIPTION TABLES "
.pp
The number of lock modes is defined by the value MAXLOCKTYPES.
The internal values of the lock modes are integers in the range 
1..(MAXLOCKTYPES-1) with zero being the value of the unlocked mode, NL.
These integers are
interpreted as different lock modes like S, X, IX, etc., by the person
designing the lock tables. An example definition for the hierarchical locks
defined in [GRAY] is given in Fig. 1.
.pp
.br
.br
.ls 1
.ft R
.sp 1
.in+16
.br
.IP
#define MAXLOCKTYPES 6
.sp 1
.br
.IP
#define NL 0
.br
.IP
#define IS 1
.br
.IP
#define IX 2
.br
.IP
#define S 3
.br
.IP
#define SIX 4
.br
.IP
#define X 5
.in-16
.sp 1
.ce 
\fBFig. 1: Lock Modes\fR
.ft R
.ls 2
.sp 1
.pp
Several tables are needed to capture the definition and functionality of
a set of lock modes and their control strategy. The first among them is the
compatibility table. This table has an entry for each pair of lock modes
and indicates if the locks are compatible. The compatibility table is 
symmetric about the diagonal but the entire table is stored, since the
storage is small and the execution is more efficient. The following figure 
illustrates the method of defining an example compatibility table in the code.
.TS
center, tab(#) ;
c s s s s s s
c|c c c c c c
l|l l l l l l.

#NL#IS#IX#S#SIX#X
_
NL#Y#Y#Y#Y#Y#Y

IS#Y#Y#Y#Y#Y#N

IX#Y#Y#Y#N#N#N

S#Y#Y#N#Y#N#N

SIX#Y#Y#N#N#N#N

X#Y#N#N#N#N#N
.TE
.br
.ls 1
.ft R
.sp 1
.in+4
.br
.IP
#define ILLEGAL -1
.br
.IP
#define LEGAL 0
.br
.IP
short LM_compat[MAXLOCKTYPES][MAXLOCKTYPES]= {
.in+4
.br
.IP
{ LEGAL  , LEGAL  , LEGAL  , LEGAL  , LEGAL  , LEGAL  },
.br
.IP
{ LEGAL  , LEGAL  , LEGAL  , LEGAL  , LEGAL  , ILLEGAL},
.br
.IP
{ LEGAL  , LEGAL  , LEGAL  , ILLEGAL, ILLEGAL, ILLEGAL},
.br
.IP
{ LEGAL  , LEGAL  , ILLEGAL, LEGAL  , ILLEGAL, ILLEGAL},
.br
.IP
{ LEGAL  , LEGAL  , ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL},
.br
.IP
{ LEGAL  , ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL}
.in-4
.br
.IP
};
.in-4
.sp 1
.ce 
\fBFig. 2: Compatibility Table\fR
.ft R
.ls 2
.sp 1
.pp
The convertibility graph of a set of lock modes can be represented by a
convertibility table.
The convertibility table for the hierarchical locks is shown in Fig. 3
and all its values are legal. 
The rows indicate the present mode
and the columns indicate the new mode.
In this example all conversions
are allowed and the resultant lock mode
after the conversion is indicated by the entry. It is possible to change this
table to allow conversion from a stronger lock to a weaker lock eg: convert
an SIX lock to a S lock. Such conversions do occur in practice,
like in B-Tree locking algorithms [BaSc]. In such examples, the table 
will not be symmetrical about the diagonal.
.pp
Usually, the above two tables are sufficient to capture the lock modes and
locking strategy. For efficiency considerations, in order to maintain
the set of locks held on a resource at any time, it makes sense to keep
the transaction holding the strongest mode at the head of the list. Any
new request has to be checked for compatibility against only the head of the 
queue and not against all the modes. For this purpose, it must be possible to
compare two lock types that are compatible and determine which is stronger.
This table is called the supremum table and is given below for the 
hierarchical lock modes. Special cases that occur when the strongest
mode for a held resource changes during lock and unlock requests have to
be handled carefully. Counts of the number of locks of a particular lock type 
held on a resource are maintained. In this example, the convertibility
table is the same as the supremum table, since only lock upgrades are 
allowed.
.TS
center, tab(#) ;
c s s s s s s
c|c c c c c c
l|l l l l l l.

#NL#IS#IX#S#SIX#X
_
NL#NL#IS#IX#S#SIX#X

IS#IS#IS#IX#S#SIX#X

IX# IX#IX#IX#SIX#SIX#X

S# S#S#SIX#S#SIX#X

SIX#SIX#SIX#SIX#SIX#SIX#X

X#X#X#X#X#X#X
.TE
.br
.ls 1
.ft R
.sp 1
.in+4
.br
.IP
short LM_conv[MAXLOCKTYPES][MAXLOCKTYPES]= {
.in+4
.br
.IP
{ NL, IS, IX, S, SIX, X },
.br
.IP
{ IS, IS, IX, S, SIX, X },
.br
.IP
{ IX, IX, IX, SIX, SIX, X },
.br
.IP
{ S, S, SIX, S, SIX, X},
.br
.IP
{ SIX, SIX, SIX, SIX, SIX, X},
.br
.IP
{ X, X, X, X, X, X}
.in-4
.br
.IP
};
.in-4
.sp 1
.ce 
\fBFig. 3: Convertibility Table (Supremum Table too for this example).\fR
.ft R
.ls 2
.sp 1
.pp
The strategy of holding the strongest mode of a set of compatible
lock modes at the head of the queue(grant list) will work only
when the supremum exists. If in a locking system, there exist compatible
lock modes whose strengths cannot be compared, then the lock manager 
has to compare the mode of every request with all the modes currently held on 
the resource. In the hierarchical lock example, the modes S and IX are
incomparable but since they are incompatible modes, the strategy works.
The lock manager has code to handle both cases. The mode of operation
can be specified by a flag during compile time.
.pp
Apart from the above mentioned tables, other tables are maintained like one 
to convert lock modes into names and another to indicate if a lock
mode is a leaf type lock or not. The first table can be used to convert the
integer value for a lock mode to a user identifiable form like X, SIX, etc.,
while producing lock manager traces.
The second table can be used to ensure,
in a hierarchy, that a correct lock is taken at the leaf level. 
Leaf level locks can however be acquired at any level of the hierarchy.
.sh 1 "LOCK MANAGER INTERFACE"
.pp
The formats of the lock and release calls are shown in Fig. 4. The locker has to
provide a valid transaction identifier, the resource identifier, the mode,
the duration and whether the lock is conditional or not. The duration
can be of three types: instant, manual, or commit duration. There are two 
levels of lock granularity, the file and the page.
A transaction can be started using
the function call \fIactivate()\fR. Lock and unlock calls can be made 
subsequently followed by a commit or an abort call. If a deadlock is detected,
the lock manager chooses the transaction that submitted the latest request 
as the victim for abort. Only manual duration locks should be released before commit or 
abort. The lock manager does not perform any checks and it is up to the
application not to prematurely release commit duration locks.
.sh 1 "STATUS"
.pp
The lock manager now works with the interface described in the previous
section. The tables have to be statically defined at compile time.
Currently, the resource formats are unchanged from the Gamma version. 
For use in Exodus, the formats have to be changed to the file and page
formats used by the Exodus storage manager. The handling of resource comparisons
is done orthogonal to the locking part and hence these changes should be
possible without much difficulty. Finally, a set of changes to the 
synchronization primitives are required. Currently, semaphore operations are 
performed by using the NOSE library [GHAN].
.pp
The lock manager has been tested in single user mode on
the VAX8800 and also been tested in multi-user mode to a limited extent 
on the hypercube. Since there is no virtual memory memory on the hypercube, 
a very nasty bug involving \fImalloc()\fR showed up and was fixed. This bug
had existed from the old Gamma version but it did not appear in even prolonged
runs on the VAX8800.
.br
.ls 1
.ft R
.sp 1
.in+4
.br
.IP
int activate (trans_id)
.in+4
.br
.IP
int    trans_id;
.br
.IP
/* Given a transaction id, a cell is created in the wait for graph */
.in-4
.sp 1
.br
.IP
struct    graph_bucket    *find_trans (trans_id)
.in+4
.br
.IP
int     trans_id;
.br
.IP
/* Given a transaction id, it finds the node corresponding to the transaction
in the transaction wait for graph */
.in-4
.sp 1
.br
.IP
lock_file (trans_cell, file_id, mode, duration, cond)
.in+4
.br
.IP
register struct graph_bucket *trans_cell;/* cell in the wait for graph*/
.br
.IP
FID file_id;
.br
.IP
LOCKTYPE   mode;
.br
.IP
DURATION   duration;
.br
.IP
boolean     cond;
.br
.IP
/*  Given a file, it tries to acquire the desired lock mode on it.
Return values are GRANTED, NOT_GRANTED and ABORTED */
.in-4
.sp 1
.br
.IP
lock_page (trans_cell, page_id, mode, duration, cond)
.in+4
.br
.IP
register  struct      graph_bucket    *trans_cell;
.br
.IP
PID page_id;
.br
.IP
LOCKTYPE   mode;
.br
.IP
DURATION duration;
.br
.IP
boolean cond;
.br
.IP
/* Given a page, it tries to acquire the desired lock mode on it.
Return values are GRANTED, NOT_GRANTED and ABORTED */
.in-4
.sp 1
.br
.IP
m_release_page(trans_cell, page_id)
.in+4
.br
.IP
struct    graph_bucket *trans_cell;
.br
.IP
PID    page_id;
.br
.IP
/* Release a manual lock on a page */
.in-4
.sp 1
.br
.IP
m_release_file (trans_cell, file_id)
.in+4
.br
.IP
struct    graph_bucket  *trans_cell;
.br
.IP
FID    file_id;
.br
.IP
/* Release a manual lock on a file */
.in-4
.sp 1
.br
.IP
committ_trans (trans_id)
.in+4
.br
.IP
int    trans_id;
.br
.IP
/* Given a transaction it releases all the resources it acquired during its 
life */
.in-4
.sp 1
.br
.IP
abort_trans (trans_id)
.in+4
.br
.IP
int    trans_id;
/* Given a transaction it is aborted by releasing all the resources it acquired */
.in-8
.sp 1
.ce 
\fBFig. 4: Lock Manager Interfaces
.ft R
.ls 2
.sp 1
.pp
The overall code size in the new lock manager is about 300 lines less than
that in the earlier version.
This reduction happened in spite of the extra 
features introduced in the new lock manager like handling
of lock upgrades, conditional locks, etc.
The parametrization of the lock modes eliminated the
number of special cases and thus simplified the lock manager.
The old lock manager used an extensive set of macros to 
achieve acceptable performance and these have been maintained intact in the modified
version. Changes made to the resource management part of the code may
involve modifications of these macros.
Simple lock calls (not including upgrades)
are executed by similar code in the new system and there should not
be any reduction in performance for them. Lock upgrades will be slower
but they are handled as efficiently as possible.
.br
.sp
\fBREFERENCES\fR
.ls 1
.sp 1
.br
[GHAN] Ghandeharizadeh, S. and DeWitt, D. J., \fBExtensions to WISS for
Supporting Multiple, Concurrent Users\fR,
University of Wisconsin, Madison.
.br
.sp 1
[MoLe] Mohan, C. and Levine, F.,\fB ARIES/IM: An Efficient and High Concurrency
Index Management Method Using Write-Ahead Logging\fR, IBM Research Report
RJ 6846, IBM San Jose Research Laboratory, August 1989.
.br
.sp 1
[BaSc] Bayer, R. and Schkolnich, M., \fBConcurrency of Operations on B-Trees\fR,
Acta Informatica, Vol. 9, No. 1, p1-21, 1977.
.br
.sp 1
[GRAY] Gray, J., \fBNotes on Data Base Operating Systems\fR, 
IBM Research Report RJ 2188, IBM San Jose Research Laboratory, February 1978.
