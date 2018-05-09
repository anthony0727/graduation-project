.bp
.po 1.25i
.(b
.sp 10
.)b
.ps 12
.ce 100
Design Specifications for the Wisconsin Storage System

Version 4.0

.ps 10
Computer Sciences Department
University of Wisconsin
Madison, WI 53706


.pn 1
.bp
.po 1.25i
.fo 'WiSS Design Specifications'-%-'1990 February'
.ce 5
Design Specifications for the Wisconsin Storage System

Computer Sciences Department
University of Wisconsin
Madison, WI 53706

.sh 1 "System Overview"
.pp
In this document, we describe a preliminary architecture for the Wisconsin
Storage System (henceforth known as WiSS).
The system's primary design goal is to serve as a flexible test-bed for
experimental database system implementation.
.pp
The system provides sequential files, unstructured files,
indices, and long data items.
A sequential file is a sequence of records.
Records may vary in length, and may be inserted and deleted at arbitrary
locations within a sequential file.
An index is always associated with a sequential file.
The index maps key values to the records of the sequential file that
contain a matching value.
Unstructured files are streams of bytes, provided to imitate files under UNIX.
A long data item is also a sequence of bytes.
However, insertion and deletion at arbitrary locations is supported
on long data items.
.pp
A Record Identifier, or RID, is a triplet containing a volume identifier,
page address, and record number within the page.
It uniquely identifies a record, as well as giving its physical address.
Indices are constructed from B-trees (actually B+trees,
with <key, RID list> entries in the leaves).
In a index, the key is a field value, and the RID list consists 
of the RIDs of records within the primary file who have that key value. 
For dense indices, there is one key per record in the primary file, with
precisely one RID in the RID list.
.pp
The system as currently architectured consists of four distinct layers.
The lowest level, level 0, deals with the aspects of physical I/O.
It is built on top of raw disk devices, although it should be possible 
to build an interface to a conventional file system.
The next level, level 1, is the buffer manager, and uses the read and
write operations of level 0 to provide buffered I/O to higher levels.
Level 2 is the storage structure level, and implements B+trees
and sequential files.
It maps record references into the appropriate page accesses, which
are buffered by level 1.
Finally, level 3 is the access methods, which 
provide the primitives for scanning a file via sequential scan or indices.

.so design.0
.so design.1
.so design.2
.so design.3
.so locks.nr
