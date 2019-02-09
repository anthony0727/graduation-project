.ig
last edited by
chou (4/1/84)
..
.sh 1 "The Access Method Level"
.(x
.sh 1 "The Access Method Level"
.)x
.pp
Level 3 implements the access methods of sequential scan, index scan,
and long item scan.
For sequential scans and index scans, a boolean expression can be
specified as a search predicate.
Only records that satisfy the boolean expression are visible during the scan.
In addition, a pair of keys can also be specified as the range 
for a B-tree index scan.
This allows for efficient range scans through indices.

.sh 2 "Scan Table"
.(x
.sh 2 "Scan Table"
.)x
.pp
Level 3 software maintains an active scan table that describes all the scans
in progress. 
For each scan, the table contains the scan type (sequential,
index, or long), and the open file number(s) of the file(s) 
associated with the scan.
For scans on long data items, a cursor is kept in the scan
table which is the current byte of the scan.  
The cursor is a RID for a sequential scan and a pointer to a B-tree
entry for an index scan.
A few parameters exist in the scan table which are determined by the scan type.
For example, a boolean expression is kept in the table for a sequential
scan or an index scan.
.sh 3 "Scan Table Structure"
.(x
.sh 3 "Scan Table Structure"
.)x
.pp
This scan table is a set of records, each of which describes a 
currently open scan on some open file.
Records are organized into dynamically allocated blocks.
The number of blocks which can be allocated is unlimited, and a pointer 
to each block is recorded in an array of block pointers.
When the table is first created, a small pointer array with one table block
are allocated.
The first pointer in the array is to the first table block; the rest are NULL.
As new blocks are allocated their addresses are recorded in the pointer array.
If the pointer array is full when a new block needs to be allocated, 
a larger array is allocated and the old array is copied into it, 
providing empty spaces at the bottom.
The old table is deallocated.
.pp
An integer scan ID is used to identify a particular scan.
An index into the table resembles segmentation:
the last N bits of the integer is the index within any block, and the other 
bits are the index into the pointer array to choose the correct block.
Scan table entries are marked as free with the NIL in the file number field.
Each table entry also contains a link field which is used to create "chaining"
of table entries, both free and used.
A free list pointer points to the top of the chain of unused table entries.
In this way free entries within a block are chained together when 
a block is allocated.
To add a new scan to the table, an entry is taken out from the free list.
The entry is then initialized and added to the chain of scans that belongs to
the same file.
If no free entries exist, a new block is allocated.
.pp
Scan table entries in use are grouped according to the file on which the 
scans were opened.
Small chains exist for each of these sets, and there is an array contains 
the scan ID of the last scan open on that file.
To delete a currently open scan, take it out of the file chain and place the 
table entry at the beginning of the free list.
.pp
An example of scan table organization with 6 currently open scans on 2 files
is shown in Figure \n+(FG.
Three dynamic scan table blocks have been allocated.
To index this table with a scan ID, the integer scan ID is broken into 
two parts: the pointer array index and the block index.
The last N bits of the scan ID are the block index, 
N is 2 in the following example,
and the other leading bits are the pointer index.
Thus, in the example, a scan ID=9 (binary 1001) 
has pointer index 2 and block index 1.
One "chain" of scans is produced by the 4 scans open on file 10.
To find this chain, use the file number (10) as the index into the pointer
array, and get the scan ID of the last scan open on that file (5).
The "next" of scan ID 5 is scan ID 0. 
This creates a chain from 5 to 0 to 2 to 3 to 5.
This order need not be sequential.
Other things to notice in this example:
(1) The next of scan ID 11 ( the last table entry allocated ) is -1 to indicate 
end of free list.
(2) At least 9 scans were open within this scan table, or the 3rd table
block would not have been allocated.

.sh 3 "Scan Table Routines"
.(x
.sh 3 "Scan Table Routines"
.)x
.de RR
.np
\\$1 (in \\$2.c)
.sp 0
..
.RR "AM_addscan" "AM_scantable"
Find an free table entry in the scan table, initialize 
it and return its ID (index).
.RR "AM_dumpscantable" "AM_scantable"
Print the scan table to aid in debugging.
.RR "AM_getscan" "AM_scantable"
Convert a scan ID into a memory address that points to an entry in
the open file table.
.RR "AM_initscantable" "AM_scantable"
Initialize the scan table at WiSS startup.
.RR "AM_nextscan" "AM_scantable"
Return the ID of the next scan, in the scan group of a particular file,
that follows a given scan ID. 
Returns NIL if the specified scan ID is the last one on the list.
If NIL is given as the current ID, the first scan open on the file is returned.
.RR "AM_removescan" "AM_scantable"
Delete a scan from the scan table and return its space to the free list.

.sh 2 "SARGable Predicates in WiSS"
.(x
.sh 2 "SARGable Predicates in WiSS"
.)x
.pp
A "SARGable predicate" is a "predicate usable as a Search ARGument."
It is a Boolean expression in
conjunctive normal form, consisting of zero or more terms
which are semantically ANDed together to determine whether a record
meets all the criteria of membership in a scan.
Only records for which all the terms of a SARGable predicate are true 
are visible in the scan.
The OR operation is not implemented in WiSS at level 3.
Any SARGable predicate containing an OR must be divided into
two or more subpredicates, and records that satisfy it must be
sought in two or more scans, one for each subpredicate.

.sh 3 "Internal Structure of SARGable Predicates"
.(x
.sh 3 "Internal Structure of SARGable Predicates"
.)x
.pp
Each SARGable predicate consists of a linked list of structures,
each of which contains one term of the complete Boolean expression.
Each term contains:
.np  
A relational operator (equal to, not equal to, greater than, 
greater than or equal to, less than, less than or equal to) 
by which the field is to be compared to the given argument.
.np
A field descriptor of the form <data type, data length, offset into record>
that describes the attributes of the field.
.np
A constant argument to which the field is to be compared.
.np
A pointer (memory offset in bytes) to the next boolean term. 
It is NULL for the last term on the list.

.sh 3 "Notes on Boolean Terms"
.(x
.sh 3 "Notes on Boolean Terms"
.)x
.pp
Any relational operator may be applied to any data type.
The data types of the field and the argument to which
it is compared must be identical, or strange results may
occur.  Likewise, the data type field must indicate
.ul
exactly
the type of the field and the argument.
There is a 256 character limit on the size of a string argument.
Conceivably, it would be possible to search for a longer
string by using two or more terms and adjusting
the offset appropriately.
.pp
The alignment of numeric fields within a WiSS record does
not matter.  The routine that evaluates SARGable predicates
allows for the fact that numeric data may not be stored on
normal boundaries.

.sh 3 "Routines"
.(x
.sh 3 "Routines"
.)x
.RR "AM_apply" "AM_boolean"
This routine evaluates a SARGable predicate.
It expects a pointer to a RID and a pointer to a boolean expression as 
input parameters.
FALSE is returned if any term is false for this record; TRUE otherwise.
(The Boolean expression pointer may be NULL; in this case, the
expression is considered true.)
.RR "AM_dumpboolean" "AM_boolean"
This routine print a boolean expression in a readable form.

.ne 36
.nf
.cs R 24
.tr -\(em

    block
pointer array      scanid next  file   information
                               number
 |-----|          |-----|-----|------|--------------------|
 |  0  | -------> |   0 |  2  |   10 |                    |
 |  1  | ------   |   1 |  6  |   11 | (scan information) |
 |  2  | ---- |   |   2 |  3  |   10 |                    |
 |  3  | -+ | |   |   3 |  5  |   10 |                    |
 +-----+  | | |   |-----|-----|------|--------------------|
          = | |   |-----|-----|------|--------------------|
            | --> |   4 |  7  |   -1 |                    | 
            |     |   5 |  0  |   10 | (scan information) |
            |     |   6 |  1  |   11 |                    |
            |     |   7 |  8  |   -1 |                    |
            |     |-----|-----|------|--------------------| 
            |     |-----|-----|------|--------------------|
            ----> |   8 |  9  |   -1 |                    |
                  |   9 | 10  |   -1 | (scan information) |
                  |  10 | 11  |   -1 |                    |
                  |  11 | -1  |   -1 |                    |
                  |-----|-----|------|--------------------|

                                  file chain array: (nonempty) 
                                  file number  scanid
                                              |------|
                                       10 --> |   5  |
                                       11 --> |   6  |
                                              |------|


.ce
Figure \n(FG. Scan Table Example 


.cs R
.tr --
.fi

