.ds f. storage.l

.ig
last edited by
chou (1/25/83)
..
.sh 2 "Long Data Items"
.(x
.sh 2 "Long Data Items"
.)x
.pp
One of the goals of WiSS is to facilitate storage of variable length items
which may grow to unlimited size
(in particular, much larger than a single page).
WiSS calls such items
.ul
long data items.
A long data item is intended to be used as a field inside a record.
Since a long data item is usually too large to fit in a normal record,
a RID, instead of the data, indicating the
location of the long data item is stored in the record that contains it.
.pp
A long data item consists of two parts: 
a collection of data segments called 
.ul
slices,
and a directory to these data segments.
Each slice is at most one page in length.
It is implemented by creating a normal record whose length 
is the maximum size of the usable space on a data page.
(Therefore, when a slice is created, it will be the only entry on the page.)
If a data item needs only a small fraction of its last segment,
efficient use of the space becomes a concern.
We define a 
.ul
crumb
to be a segment of a long data item which is much small than a page.
A crumb is implemented as a short record that 
shares space with other crumbs or ordinary records.
.pp
The
.ul
directory
of a long data item
exists as an ordinary record that may grow up to the size of a page.
Inside the directory is the
RID and the length of each data segment of a long data item.
This provides fairly efficient access to an arbitrary section
within a long data item.

.ne 4
.TS
.if \n+(b.=1 .nr d. \n(.c-\n(c.-1
.de 35
.ps \n(.s
.vs \n(.vu
.in \n(.iu
.if \n(.u .fi
.if \n(.j .ad
.if \n(.j=0 .na
..
.nf
.nr #~ 0
.if n .nr #~ 0.6n
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.fc
.nr 33 \n(.s
.rm 80 81 82 83 84
.nr 80 0
.nr 38 \w# of bytes
.if \n(80<\n(38 .nr 80 \n(38
.80
.rm 80
.nr 81 0
.nr 38 \w# of segments
.if \n(81<\n(38 .nr 81 \n(38
.81
.rm 81
.nr 82 0
.nr 38 \wRID 1 / Length 1
.if \n(82<\n(38 .nr 82 \n(38
.82
.rm 82
.nr 83 0
.nr 38 \w...
.if \n(83<\n(38 .nr 83 \n(38
.83
.rm 83
.nr 84 0
.nr 38 \wRID n / Length n
.if \n(84<\n(38 .nr 84 \n(38
.84
.rm 84
.nr 38 1n
.nr 79 0
.nr 40 \n(79+(1*\n(38)
.nr 80 +\n(40
.nr 41 \n(80+(3*\n(38)
.nr 81 +\n(41
.nr 42 \n(81+(3*\n(38)
.nr 82 +\n(42
.nr 43 \n(82+(3*\n(38)
.nr 83 +\n(43
.nr 44 \n(83+(3*\n(38)
.nr 84 +\n(44
.nr TW \n(84
.nr TW +1*\n(38
.if t .if \n(TW>\n(.li .tm Table at line 55 file storage.l is too wide - \n(TW units
.ne 1v+0p
.fc  
.nr #T 0-1
.nr #a 0-1
.nr #a 0-1
.eo
.de T#
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.mk ##
.nr ## -1v
.ls 1
.if \n(#T>=0 .nr #a \n(#T
.if \n(T. .vs \n(.vu-\n(.sp
.if \n(T. \h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.if \n(T. .vs
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'|0'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'(|\n(41u+|\n(80u)/2u'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'(|\n(42u+|\n(81u)/2u'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'(|\n(43u+|\n(82u)/2u'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'(|\n(44u+|\n(83u)/2u'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'|\n(TWu'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'
.ls
..
.ec
.nr 36 \n(.v
.vs \n(.vu-\n(.sp
\h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.vs \n(36u
.mk #a
.ta \n(80u \n(81u \n(82u \n(83u \n(84u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'# of bytes\h'|\n(41u'# of segments\h'|\n(42u'RID 1 / Length 1\h'|\n(43u'...\h'|\n(44u'RID n / Length n
.fc
.nr T. 1
.T# 1
.35
.nr #a 0
.TE
.if \n-(b.=0 .nr c. \n(.c-\n(d.-4
.pp
With the sizes of various addresses (RIDs, PIDs, etc.),
the directory of a long data item can accommodate up to about
400 RID/length pairs.
With the maximum capacity of 4K bytes per slice, this gives an upper 
limit near 1.6M bytes for a long data item.

.sh 3 "Updating Long Data Items"
.(x
.sh 3 "Updating Long Data Items"
.)x
.pp
When bytes are to be inserted into a long data item, the 
directory is checked to see if the new bytes can fit in place on 
the current slice (the slice indicated by the cursor position).
If there is not enough free space on the current
slice, then check the next slice and allocate new slices as needed.
At any rate, local compression is done whenever feasible.
Page fill factor is taken into consideration in this process.
After the insertion, each slice is normally as full as the page fill factor
indicates.
Slices at both ends of the insertion, however, may contain more bytes to
save space and/or reduce IO accesses.
.pp
Deletion of data from a long data item consists of deleting the data 
from each slice that lies within the range of
data to be deleted, and modifying the directory to reflect
the change in length of each slice.
Any slice whose length is reduced to 0 is freed and its
directory entry is removed.
Slices at both ends of the deletion may be merged to
save space and/or reduce IO accesses.
.pp
Compressing a long data item increases the amount of data on each slice
to at least as full as the page fill factor of the file the item belongs to;
thereby keeping the number of pages used by a long data item to a minimum.
The compression is done as follows:
Starting with the first two slices of the long data item, slide as much
data as possible from the second slice to the first.  Continue in this fashion
until data has been "shifted forward" as far as possible.  
After all the slices are "compressed" in this manner, 
free all the empty slices and compress the directory.

.sh 3 "Recovery Issues"
.(x
.sh 3 "Recovery Issues"
.)x
.pp
Long data items are problematic for recovery mechanisms.
If we define cursors on long data items, the following operations suffice:
.sp
.in 4
.ti 2
- Move the cursor to some absolute or relative location (either skipping
data or returning it to the user)
.ti 2
- Insert data before (or after) the cursor location
.ti 2
- Delete data at the cursor location
.ti 2
- Replace data at the cursor location (actually just a combination of
insert and delete)
.in 0
.pp
It is fairly straightforward to log each of these operations: if a large
segment of data is involved, it is logged in pieces small enough to fit
onto a log page.
Note that this means that when a long data item is first written, 
a great deal of information may be written to the log.
Unfortunately, it also means that writing a log record is not an atomic action.
For long data items, it is worthwhile considering whether or not
logging is desirable.
For example, changes to a letter probably don't need to be logged.
.pp
To actually update a long data item,
a variant of System R's shadowing may be used.
Instead of updating pages of a long data item in place, we allocate new pages
to hold data, and build a new directory entry in a buffer.
When we have
finished writing the data, we replace the directory (all at once, as an
atomic operation) and then free the pages replaced (not necessarily an
atomic operation).
.pp
If locking mechanisms are used in WiSS, it should probably suffice to
lock the RID of a long data item which has fields on which a predicate could
be based.
It is uncertain whether optimistic methods could be used
with long data items, since redoing an operation requires a large
amount of information to be regenerated.

.sh 3 "Routines"
.(x
.sh 3 "Routines"
.)x
.pp
.de RR
.np
\\$1  (in \\$2.c)
.sp 0
..
.RR "r_dumplong" "r_dumplong"
Print a long data item in a readable form, assuming the data is a ASCII string.
This is intended for debugging purpose only.
.RR "r_expandcrumb" "r_crumb"
Expand a crumb into a slice.
To expand a crumb, a new slice is allocated to inherit all the data bytes
in the crumb, and the crumb (which is actually a short record) is
simply deleted.
The RID of the crumb (in the directory)
is replaced by the RID of the new slice after the expansion.
.RR "r_initslice" "r_initslice"
Given a page, initialize it into a slice.
A page buffer is allocated first, 
and then a huge record that takes up the entire free
space on the page is created.
.RR "r_shrinkslice" "r_crumb"
Shrink a slice into a crumb.
To shrink a slice, a short record is created to store all the 
bytes on the slice, then the resources (a level 0 page and a level 1 buffer)
used by the slice is release.
The RID of the slice in the directory is replaced by the 
RID of the new crumb.
.in 0

