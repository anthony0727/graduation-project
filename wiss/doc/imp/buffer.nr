.ig
last edited by
chou
..

.sh 1 "The Buffer Management Level"
.(x
.sh 1 "The Buffer Management Level"
.)x
.pp
Level 1 is responsible for buffer management,
and calls the level 0 interface to do the actual reads and writes of pages.
It maintains a pool of buffer pages, 
and implements a page replacement algorithm.
The replacement algorithm is a modified global CLOCK algorithm that 
incorporates a FIX/UNFIX protocol.
Certain interface routines implicitly fix a page in the buffer pool.
Fixed pages are exempt from page replacement considerations and are "unfixed"
through the use of some other interface routines.
The buffer manager's primary data structure is the buffer table, 
an in-core table
that keeps track of the state of pages that reside in the buffer pool.
The buffer table is implemented as an array of records containing the
following information:
.(l
File identifier: the open file number of the file to which the page belongs.
Page identifier: ID of the page resides in this buffer.
Fix count: the number of users fixing this page.
Valid bit: status bit that tells whether the buffer contains a valid page.
Modified bit: status bit that tells whether the buffer has been modified.
Reference bit: status bit that tells whether the buffer has been accessed.
.)l
.pp
A hash table keyed on page identifier is used to reduce the search time 
of a page in the buffer pool.
Entries in the hash table are indices to the buffer table, where the
status information on buffers is stored.

.sh 2 "Routines"
.(x
.sh 2 "Routines"
.)x
.de RR
.np
\\$1  (in \\$2.c)
.sp 0
..
.RR "BF_allocbuf" "BFallocbuf"
This routine carries out the swapping algorithm and returns the index of
a free buffer to the caller.
.RR "BF_delete" "BFhash"
Delete an entry from the hash table.
.RR "BF_dumpbuftable" "BFdumptables"
This dumps the entire buffer table (for debugging purpose).
.RR "BF_init" "BFhash"
This initializes the hash table for the buffer pool.
.RR "BF_insert" "BFhash"
Insert a new entry into the hash table.
.RR "BF_lookup" "BFhash"
Look up a page in the buffer pool through the hash table.

