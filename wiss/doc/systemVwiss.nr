.po 1.0
.ll 6.5i
.ls 1
.he ''%''
.po 1.0i
.ce 10
\fBArchitectural Overview of Multiuser WiSS under System V Shared Memory\fR
.sp
David J. DeWitt
Computer Sciences Department
University of Wisconsin
Madison, WI 57306
.sp 
November, 1990
.sp 2
.ce 0
.fi
.sh 1 "Introduction and Key Assumptions"
.pp
The basis of the System V version of Wiss (which we shall designate
as WiSS-V) was the multiuser version of WiSS used in the 
Gamma database machine (which we shall designate WiSS-G).
WiSS-G was designed to operate in a special
operating system environment call NOSE.
The basic services provided by NOSE are non-preemptive threads 
running in a shared address space, semaphores, communications
primitives for sending messages between processors, and a rundamentary
disk driver.
Semaphores in NOSE are very cheap to use (less than 20 instructions)
and basically unlimited in quantity.
The key considerations when moving this version of WISS from NOSE
to System V Unix were:  a) semaphores under System V are very
expensive (1000s of instructions),  b) processes do not share
an address space and c) scheduling is no longer non-preemptive.
Thus, a process can be suspended at any time while holding a
critical latch.  In the following sections we describe
the solutions we adopted during the design and implementation
of WiSS-V.
.br
.sh 1 "Semaphores and Latches"
.pp
The first decision that we were forced to deal with was how
to handle semaphores.  WiSS-G uses hundreds of semaphores
- more than any System V operating system can be expected to
support gracefully.  Second, in very critical places 
in the code (such as removing an element from a linked list)
a WiSS-G thread would simply raise its priority - in effect
turning interrupts off until the critical operation was completed.
Such an approach was simply not possible in the design of WiSS-V.
.pp
The solution we adapted was to replace the NOSE semaphore mechanism
with a mechanism we termed a "latch".  Basically, the way
a latch works is as follows.  A latch can be in two states "busy" 
or "free" and two processes cannot hold the same latch
simultaneously.  Assume initially that latch L is free.  For a process
to set a latch it first enters a critical section around the latch
via a hardware "test and set" instruction on the field "sync" of the latch.  
Once inside the critical section it examines the current state of the latch.
If the latch is free, it sets the latch, and then exits the
critical section.  If, on the other hand, the latch is busy
the process adds itself to a list of waiting processes
associated with the latch.  After adding itself to this list, the
process exits the critical section and then goes to sleep
on its own System V semaphore.  When  a process holding a latch
is about to release the latch it again enters the critical section
around the latch and then looks at the list of waiting processes
to see if it is empty. 
If the list is empty (ie. no processes are waiting for the latch),
the process simply marks the latch "free" and exits from
the critical section.  
If the list is not empty, the processes wakes up the first process
on the wait list (via a System V semaphore wake up call), marks
the latch "free" and exits the critical section.  The process
which has been awakened, marks the latch "busy" and exits
the critical section.
.pp
The definition of the type LATCH is found in the file latch.h.
Three operations are defined on a latch. InitLatch(), SetLatch(),
and ReleaseLatch().  InitLatch() is used to initialize a latch,
SetLatch() is used to acquire exclusive access to a latch and
ReleaseLatch() is used to release a latch.  The code for
these operations can be found in SM/latch.c.  When examining
latch.h and latch.c it is important to keep in mind that
neither the MIPS 2000 or 3000 CPU has a hardware test and set
instruction and thus a different technique is used to control
access to the critical region.  The make files
automatically determine what the target architecture is,
choosing the correct latch definition and assembly language
test and set instruction.  The assembly language "test and set"
instructions for the Sun 3, Sun 4, 386, and Vax processors
are contained in SM/testandset.S.
.pp
As discussed above,  each process must have its own System V semaphore
which it uses to put itself to sleep when it cannot acquire a latch.
The file SM/semaphore.c contains four procedures for manipulating
System V semaphores.  InitSem() acquires a System V semaphore
via a semget() call and initializes it via a semctl() all.
RmSem() is used to remove a semaphore.  The operation WaitSem() 
(the "P" operator) is used by a process to wait on a semaphore 
and the operation SendSem() (the "V" operator)  is used by 
one process to signal another.   As each process starts
up it acquires one semaphore during the initialization phase
(which will be discussed in more detail below).
.pp
.sh 1 "Use of Shared Memory"
.pp
Since processes under Unix (unlike NOSE) do not share an address
space we were forced to use the System V shared memory mechanism
in order to share critical WiSS resources such as the buffer
pool and lock table among multiple, concurrently executing
processes (there are other shared structures as we will
address below).  Before describing all the shared structures
in more detail, we begin instead by describing the
how the shared-memory segment is acquired, initialized,
and accessed.
.pp
To begin with, the organization of the shared memory segment
is defined by the structure SMDESC found in the file sm.h.
In some cases (e.g. the users[] structure at its beginning)
the shared data structures are declared as part of SMDESC
and, hence can be accessed directly.  In others, the
shared structure (e.g. the buffer pool - "PAGE *bufferpool;")
is "malloced" out of the shared memory heap at initialization
time and is accessed via a pointer.  Fields of the shared memory
structure are accessed via the pointer variable \fBsmPtr\fR
which is initialized when the process is created.  
.pp
Initialization of the shared memory segment is performed
by the routine sm_init() in the file SM/sm_init.c.  
Initialization is a tricky process as two
processes that start simultaneously cannot both be allowed
to initialize the shared memory segment.  To prevent this case, WiSS-V
uses the Unix flock() command to set an exclusive lock on 
the Unix file SHM_KEYFILE (declared in sm.h).   After
a process acquires the lock and performs some initial
calculations (ie. determining the size the segment should be)
it then gets the shared memory segment (via a shmget()
call) and attaches itself to the shared memory segment
(via the call \fIsmPtr = shmat()\fR).
It then uses the shmctl() call to get status information
on the segment.  In particular, if it finds that it is the only
process attached to the segment, it then proceeds to initialize
all the fields of the SMDESC structure, "mallocing" via
shmAlloc calls (defined in util/shmalloc.c) structures
such as the buffer pool.  Finally, the process  acquires
a semaphore and then releases the lock on the file SHM_KEYFILE.
.pp
When a WiSS process is about to exit it calls wiss_final() (declared
in SM/sm_init.c).  This routine also use flock() to get
an exclusive lock on SHM_KEYFILE.
Once the lock has been acquired, the process deletes
its own System V semaphore and then it determines how
many other processes remain attached to the shared memory
segment.
If it determines it is the last one attached, it
calls sm_final() (in SM/sm_final.c) to flush critical 
information such as the Volume and File Tables to disk and
then it releases the shared memory segment.  
Finally, it releases the lock on the SHM_KEYFILE so
that other processes can start.  
.br
.sh 2 "Buffer Pool Organization"
.pp
The type definitions for the buffer pool and its associated control
structures are contained in sm.h  There are four principle structures.
The buffer pool itself (PAGE *bufferpool), the buffer pool
control table (BUFTAB  *buftable), a hash table used
to determine whether a particular page is in the buffer
pool (HASHBUCKET *hashTable), and a latch table used for
short term latches on pages for the duration of a single
buffer pool operation (LOCKBUCKET PageLocks[LOCKTABLESIZE]).
.pp
The buffer pool is malloced from shared memory in sm_init()
by the first process to attach itself to shared memory.  It size
is determined by MAXBUFS which is declared in wiss.h.  
Replacement of buffer pool pages is coordinated
by the \fIbuftable\fR structure.
This structure contains information on whether a page is fixed
or not, whether it is dirty, etc.   Allocation of a buffer
pool page is performed in the routine 2/BFallocbuf.c using
a clock (i.e. pseudo LRU) replacement policy.
In order to maximize performance, allocation of 
buffer pool pages is not strictly serialized with
a single latch.  Instead, processes needing a free page
search the buftable in parallel (starting at different spots).
A latch in each buftable entry is used to insure that two
processes don't both allocate the same buffer pool frame.
.pp
The structure \fIHASHBUCKET *hashTable\fR is used in conjunction
with the three routines, BF_lookup(),  BF_insert(), and BF_delete()
defined in 1/BFhash.c to provide a mapping from a page identifier (a PID)
to a buffer pool frame for disk pages currently resident in the buffer
pool.  BF_insert() adds a new PID to the hash table, BF_delete() removes
an entry, and BF_lookup(), given a PID, returns the index of the buffer pool
frame containing the corresponding page if the page is resident in
the buffer pool.  
Concurrent accesses to hashTable (which is implemented 
with chained-bucket hashing) are synchronized via a latch
on each of its hash chains.  These latches are defined by
the structure \fILATCH *hashTableLatches\fR (declared in sm.h). 
.pp
The final buffer pool structure in shared memory is the "lock" table
defined by the structure \fILOCKBUCKET PageLocks[LOCKTABLESIZE]\fR.
This lock table is used to synchronize accesses to the pages
buffer pool and to prevent concurrently executing processes
from performing unnecessary work.
Two operations are defined on this table (which is implemented
via chained-bucket hashing):  \fIBF_lock()\fR and \fIBF_unlock()\fR
(in 1/BF_hash.c).  Basically while the latches in the buftable
structure coordinate accesses at a physical level,  the PageLocks
structure provides a logical locking mechanism.  For example,
assume that process P1 needs a buffer pool frame
and that its search of buftable reveals frame i contains 
page X which is not fixed.  P1 then proceeds to set a lock on X by
calling BF_lock().  Assume that, concurrently, a second process P2 
executes a bf_readbuf() call to read page X.  The first thing
bf_readbuf() does is to call BF_lock() to set a lock on X.
There are two possible outcomes.  If P1 acquires the lock first,
it will deallocate the page from buffer pool.  If P2 acquires the
lock first, the page will be refixed and P1 will have to look
for another frame.  This lock mechanism is also used to prevent
two processes from reading the same page (e.g. an index page)
simultaneously.  The first process that sets the lock on the page
will initiate the I/O.  Any subsequent attempts to read
the page while the I/O transfer are occurring will block
until the I/O is finished and the first process releases
the lock on the page.
The locks at this level at this level are short
term locks.  Since a process holds only one such lock at a time,
there is no need for deadlock detection.  In addition, processes
hold such locks only for the duration of a single
buffer pool operation and never across buffer pool operations.
These locks are similar to the golden locks defined at the 
RSS level of system R.
Finally, it is important to keep in mind that the buffer pool 
lock table is a completely separate mechanism than the
lock manager described in the following section
.br
.sh 2 "Lock Manager"
.pp
The type definitions for the shared memory structures that are used by 
the WiSS concurrency control mechanism are contained in sm.h 
and the overall structure of the lock manager is illustrated
in Figure 1.
.(z
.GS C
sc 0.47
1 8
2 10
3 12
medium 9
file lm.grn
.GE
.sp 2
.ce 1
\fBFigure 1:  Overall Lock Manager Architecture\fR
.)z
There are two principle structures.  \fIstruct trans_bucket trans[MAXTRANS]\fR
is a hash table (implemented with chained-bucket hashing) containing
one hash bucket for each active transaction.  It is organized
by hashing on the transaction identifier of the transaction.
The type definition for \fItrans_bucket\fR is contained in
the file graph.h.  Each entry is composed of a latch and a pointer
to a \fIgraph_bucket\fR structure.  The latch is used to coordinate
accesses to the hash chain.  
The second key component of the lock manager is the hash table
(also implemented via chained-bucket hashing) 
\fIstruct locknode locktable[MAXRES]\fR.   The locktable is
organized by hashing on either a page identifier (a PID defined in wiss.h) or
a file identifier (a FID defined also in wiss.h).
Each entry (defined in lockdefs.h) contains a latch 
(again used to coordinate accesses
to the hash chain) and a pointer to a linked list of locks.
.pp
The concurrency control manager uses standard two phase lock (2PL)
protocols [GRAY78] as well as non-2PL locks for Btrees.
Two lock granularities are provided:  file-level (FID), and page-level (PID).
Deadlocks are detected using a standard
linear deadlock detection algorithm [AGRA83].  The following
sections describe operation of the lock manager (LM/lockmanager) and the 
deadlock detector (/LM/deadlock).
.br
.uh "Lock Modes"
.pp
The lock manager supports the locking of resources and manages the
transaction wait-for-graph.
Files can be locked in one of 5 modes
1) Shared (S), 2) Exclusive (X), 3) Intent
to Share (IS), 4) Intent to Exclusive (IX), or 5) Share and Intent
to Exclusive (SIX).  Pages are locked in either S or X mode.
Several tables are used to capture the definition and functionality of
this set of lock modes and the control strategy used
to coordinate lock requests by different transactions. 
The first among them is the \fBcompatibility table.\fR
This table has an entry for each pair of lock modes
and indicates if the locks are compatible.
The compatibility table is 
symmetric about the diagonal but the entire table is stored, since the
storage is small and the execution is more efficient. The following figure 
illustrates the method of defining an example compatibility table in the code.
.(z
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
.)z
.pp
The convertibility graph of a set of lock modes is represented by a
\fBconvertibility table\fR.
The convertibility table for the hierarchical locks is shown in Fig. 3
and all its values are legal. 
The rows indicate the present mode and the columns indicate the new mode.
In this example all conversions are allowed and the resultant lock mode
after the conversion is indicated by the entry. It is possible to change this
table to allow conversion from a stronger lock to a weaker lock eg: convert
an SIX lock to a S lock.  Such conversions do occur in practice,
like in B-Tree locking algorithms [BAYE77]. In such examples, the table 
will not be symmetrical about the diagonal.
.pp
Usually, these two tables are sufficient to capture the lock modes and
locking strategy.  For efficiency considerations, in order to maintain
the set of locks held on a resource at any time, it makes sense to keep
the transaction holding the strongest mode at the head of the list.  Any
new request can then be checked for compatibility against only the head 
of the queue and not against all the modes. 
For this purpose, it must be possible to
compare two lock types that are compatible and determine which is stronger.
This table is called the \fBsupremum table\fR and is given below for the 
hierarchical lock modes.
Special cases that occur when the strongest
mode for a held resource changes during lock and unlock requests have to
be handled carefully.
Counts of the number of locks of a particular lock type 
held on a resource are maintained.
In this example, the convertibility table is the same as the 
supremum table, since only lock upgrades are allowed.
.(z
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
.)z
.br
.uh "Lock Manager Operation"
.pp
As discussed above, the lock table is implemented as a hash table
organized by hashing on either the PID or FID of the resource
being locked.  As illustrated by Figure 1, the hash chain is
composed of a doubly linked list of elements of type \fInode\fR
(declared in resource.h).  For each locked resource there
is one such node.  The node contains an enumerated tag (owner)
which indicates whether the lock is on a page (owner == PAGER) or
a file (owner == FILER).   The flink and blink pointers
are used for the hash collision chain.  
In the case of a file lock, the f_page and b_page fields
of the node structure are used to form a doubly linked list of
all pages in the file that have been locked so that when the
lock on the file is released all the corresponding
page locks can be found quickly.  
The wait field is used to maintain a doubly-linked list of transactions
waiting for a lock on the resource.  Each entry in this list
is an element of type wait_node (declared in resource.h).
The o_list is another doubly linked list of transactions currently 
holding a lock on the resource (i.e., the granted group).
.pp
As illustrated by Figure 1, there is one instance of type
graph_bucket (declared in graph.h) for each active transaction.
These elements are hung off the hash table \fBtrans\fR which is organized
by hashing on the transaction identifier of the transaction.  
The flink and blink fields are used to implement the collision
chain for the hash table. 
If transaction i is waiting for a resource currently owned
by transaction j, the waiting_for field of transaction i's graph_bucket
will point to the graph_bucket of transaction j.
Likewise, the res_wait of a graph_bucket points to
the resource that transaction i is waiting for and the resource_wait
field points to the actual wait bucket.
These three fields plays a key role in detecting deadlocks among
concurrently executing transactions.
The locks_held field of the graph_bucket structure is the head
of a doubly linked list of lockq nodes, one for each lock held
by the transaction.   Each lockq node (defined in graph.h) contains
the mode of the lock (the request field) and a pointer (resptr)
to the actual lock node itself.  Note that both the lockq and node
structures are necessary because two (or more) transactions
may hold compatible locks on the same resource simultaneously.
.pp
The LM/lockmanager directory contains routines for locking (locker.c)
and releasing (release.c) resources.
Briefly, when a transaction attempts to lock a resource (a page or
a file), the lock manger checks the lock table to see if the resource
is already locked by seeing if there is an entry for the resource
in the lock table.
If not, the lock manager creates an entry in the lock table
corresponding to the resource and records 
that the transaction acquired a lock on the resource.
If a lock already exists,  then some other transaction must be holding
a lock on the resource.  The lock manager
checks the compatibility of the current lock on the resource with the 
pending lock request.  If the two locks are compatible, the lock manager
records that another lock on the resource has been acquired.
.pp
If the two locks are incompatible, then the lock manager first
determines whether a deadlock will occur if the current transaction waits
for any of the owners of the resource by calling check_deadlock()
(in LM/deadlock/detect.c)
This is done by checking whether creating an arc from the requesting 
transaction node to any of the owners of the resource (defined
by the o_list field of a lock node) will result in a cycle
in the waits-for-graph.  If so, then a dead-lock has occurred and 
the requesting\** transaction is aborted (by calling the procedure 
abort_trans(trans_id) in LM/lockmanager/trans.c).
.(f
\** Always aborting the requesting transaction is undoubtably suboptimal
and should be fixed. Better solutions are to abort the transaction
in the cycle holding the fewest locks or to abort the one that
has been running the shortest amount of time.
.)f
.pp
If no cycles are found then the transaction is allowed to wait until
the resource is unlocked.  A number of steps are taken in this case.
First, the lock manager creates an arc from the requesting transaction 
to the graph_bucket corresponding to one of the owners of the resource.
The lock manager next creates a wait-node and places it at the 
end of the list of transactions waiting to acquire the resource.  
The monitor field of the wait_node points to the System V semaphore
for the process and is used to actually block the transaction until
the resource becomes available.
The list of transactions waiting to acquire a resource is
managed using a first-come, first-served algorithm.
.fi
.ls 1
.sh 1 "Refrences"
.in 0.5i
.sp
.ti 0
[AGRA83] Agrawal, R., 
"Concurrency Control and Recovery in Multiprocessor Database Machines: Design and Performance Evaluation,"
Computer Sciences Dept., Univ. Wisconsin, Madison, Ph.D Dissertation 1983.
.sp 1
.ti 0
[BAYE77] Bayer, R. and Schkolnich, M., 
\fBConcurrency of Operations on B-Trees\fR,
Acta Informatica, Vol. 9, No. 1, p1-21, 1977.
.sp
.ti 0
[GRAY78] Gray, J. N.,
"Notes on Database Operating Systems,"
in Lecture Notes in Computer Science 60, Advanced course on Operating Systems,
ed. G. Seegmuller, Springer Verlag, New York 1978.
.br
