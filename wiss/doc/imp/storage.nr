.ig
last edited by
chou (1/25/83)
..

.sh 1 "The Storage Level"
.(x
.sh 1 "The Storage Level"
.)x
.pp
Level 2 of the Wisconsin Storage System (WiSS) manages the actual storage
of information.
It allocates room on a page for records and index entries, and performs the
operations necessary for fetching and storing records and indices on pages.
A facility for storing variable length items is also provided by this level.
Below level 2 (that is, levels 1 and 0), the basic unit of data is the page.
Above level 2, the basic unit of data is the record.
.pp 
There are three primary types of pages handled by level 2:
Data Page (containing user records),
Index Page (for storing index structures), and
Directory Page (used by this level for storing file information).
The file information is internal to the storage level, and other levels
of WiSS are not really aware of its existence (with minor exceptions at
level 0, the Input/Output Level).

.so storage.r.nr
.so storage.i.nr
.so storage.l.nr
.so storage.u.nr
.so sort.nr
.so storage.fi.nr

