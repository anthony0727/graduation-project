.\" last edited by Chou	(12/4/84)
.bp
.po 1.25i
.sp 10
.ps 12
.ce 100
User Manual for the Wisconsin Storage System

Version 4.0

.ps 10
Computer Sciences Department
University of Wisconsin
Madison, WI 53706


.pn 1
.bp

.ce 100
.po 1.25i
.fo 'WiSS User Manual'-%-' 1990 March '
User Manual for the Wisconsin Storage System

Computer Sciences Department
University of Wisconsin
Madison, WI 53706

.sh 1 "Overview"
.(x
.sh 1 "Overview"
.)x
.pp
The Wisconsin Storage System supports the following \fIstorage structures\fR:
sequential (structured) files, B-tree indices, hash indices,
unstructured files, and long data items.
A sequential file is a sequence of records.
Records may vary in length, and may be inserted and deleted at arbitrary
locations within a sequential file.
A B-tree index is always associated with a sequential file.
The index maps key values to the records of the sequential file that
contain a matching value.
Hash indices are similar to B-tree indices, except that they do not
support sequential range scans.
Long data items provide a facility for storing variable
length data objects that may grow to virtually unlimited size.
A long data item may be regarded as a simple stream of bytes.
Unstructured files are intended to imitate files under UNIX.
An unstructured file can also be viewed as a simple stream (array) of bytes.
The principal difference between unstructured files and long data items is that
the former do not support insertions and deletions at arbitrary locations.
.pp
Both sequential files and long data items can be accessed
by means of \fIscans\fR.
A scan superimposes a logical order on the components of a storage structure
and creates a \fIcursor\fR
through which access to the structure can be made.
A cursor can be moved back and forth within a storage structure according
to the logical order of the scan.
Long data items are always scanned in sequential order.
Structured files can be scanned sequentially, or
in the order specified by a B-tree index.
In addition, certain records of a sequential file can be skipped for
the purposes of a particular scan by
specifying a search predicate on field values and, in the case of a B-tree
index scan, by specifying upper and lower bounds on the index key.
Scans through hash indices allows for the retrieval of records
that has a particular key value.

.sh 1 "Data Types and Parameters"
.(x
.sh 1 "Data Types and Parameters"
.)x
.pp
This section describes the most common data types and parameters used by
WiSS interface routines.
Declarations for the data types (in C's notation) are shown in Appendix A.
We begin with data types:
.np
RID :
Record identifier - a triple consisting of a volume identifier,
page address, and record number within the page.
An RID uniquely identifies a record and also gives its physical address.
.np
KEY :
A triple <key type \**, key length, key value> that describes a key.
An example of its use is the specification of bounds on the
values to be considered in an indexed scan.
.(f
\**
The WiSS supports character strings and all scalar types
of the C programming language.
.)f
.np
KEYINFO :
A triple <key offset, key length, key type> that describes
a field inside a record.
It is used, for example, to describe the field on which a file is to be sorted.
.np
XCURSOR:
A triple <page number, slot number, offset within a slot> that
points to a RID in a slot on a particular leaf page of an index.
.np
BOOLEXP :
A linked list of simple boolean terms.
Each boolean term is of the form <argument value, relational operator,
field descriptor>, where the field descriptor is essentially the same
as a KEYINFO.
The terms are semantically AND-ed together to determine the meaning 
of the full expression \**.
.(f
\**
There is no equivalent mechanism for OR.
.)f

.lp
Common parameters to the interface routines include:
.np
DeviceName and VolID :
.sp 0
A Device Name is the symbolic name (string of characters) of an external device
on which we can mount a data storage medium.
A Volume ID is an integer that uniquely identifies a volume.
.np
FileName and OpenFileNum :
.sp 0
A File Name is a string of characters that names a file.
An Open File Number represents an active instance of a file.
A file must be open (activated) before its contents become accessible.
Open File Numbers are similar to file descriptors in UNIX.
.np
IndexNo :
.sp 0
An Index Number is used to identify an index, which is built on some
field of a sequential file.
The complete name of an index is given by
a pair <FileName, IndexNo>, where FileName is
the name of the sequential file.
.pp
Every routine returns a status code as its function value.
A negative return code means that an error occurred.
Successful execution always results in a non-negative return code \**.
.(f
\**
Some routines do return a meaningful function value.
These values are guaranteed to be greater than or equal to zero
unless an error occurred.
.)f
Routines are provided to interpret error codes.
The messages the routines produce may not be very useful to a user,
but they give a clue as to where the error originated or was detected.
.br
.sh 1 "Transactions and Locking"
.(x
.sh 1 "Transactions and Locking"
.)x
.pp
WiSS provides support for locking and transactions through the
following routines:
.np
trans_id = wiss_begin_trans();
.nf
.fi
.sp 1
Begins a new transaction and returns an integer that identifies 
the transaction.  
.np
wiss_commit_trans(trans_id)
.nf
int	trans_id;
.fi
.sp
Commits the effects of a transaction and releases all locks
held by the transaction.
.br
.np
wiss_abort_trans(trans_id)
.nf
int	trans_id;
.fi
.sp
Aborts the effects of a transaction and releases all locks
held by the transaction.  
.lp
.pp
Calls to the lock manager are performed automatically by
WiSS when the system is accessed through the level 3 interface.
A number of the lower level routines take 3 new parameters:
a transaction identifier, a \fBlockup\fR flag (a boolean), and a \fBcond\fR
flag (also a boolean).  The lockup flag turns on locking if it
is set to TRUE.  If the lockup flag is TRUE, then the conditional
flag is used by the lock manager to determine how to handle 
lock waits.  If cond is FALSE, on a conflict lock request the lock 
manager will block the requesting process until either the resource can be
granted or the lock manager detects a deadlock.  If cond is TRUE,
instead of blocking on a conflicting lock request, the lock manager will
return the value COND_WAIT.  Conditional locking is not currently used 
by any of the WiSS routines.
.pp
Certain level 3 calls expect a lockMode parameter.  The valid
lock modes are:  l_NL, l_IS, l_IX, l_S, l_SIX, l_X.  These
values are defined in the include file <lockquiz.h>.
.pp
It is \fBvery important\fR to notice that opening a file or
an index directly via the calls wiss_openfile(), wiss_openindex(),
and wiss_openhash() \fBdoes not set any locks\fR on the file
or index.  If you intend to by-pass the level 3 scan interfaces
and access records and indices directly you must \fBexplicitly\fR lock the 
file and/or index in the appropriate mode.  
Once a lock has been set on a file, the record and index routines
will automatically set the appropriate locks on the pages they
access.
The following routine can be used to set a lock on a file:
.sp
wiss_lock_file
(transId, openfilenum, lockMode, duration, cond)
.nf
int	transId;
int	openfilenum;
short	lockMode;
DURATION duration;
short	cond;
.fi
.sp
Locks the file identified by the openfilenum parameter in mode lockMode.
lockMode must be one of the valid lock modes:  l_NL, l_IS, l_IX,
l_S, l_SIX, l_X.  duration must be either COMMIT or MANUAL.  
If duration is equal to COMMIT the lock is held until the 
transaction commits.  MANUAL specifies that the lock may be 
released before the end of
the transaction.  cond is either TRUE or FALSE and indicates
whether a conditional lock is being requested (cond==TRUE).
.sp
A lock request will return one of the following values:
ABORTED, GRANTED, NOT_GRANTED, WAIT, COND_ABORTED, COND_WAIT 
(defined in lockquiz.h)
.sp
To manually release the lock on a file or index use the routine:
.np
wiss_m_release_file
(transId, openfilenum)
.nf
int	transId;
int	openfilenum;
.fi
Releases all locks held by transaction transId on file openfilenum 
and returns either OK if all went well.
.br
.br
.sh 1 "Sequential Files"
.(x
.sh 1 "Sequential Files"
.)x
.pp
A sequential file is a collection of (possibly variable-length) records.
A record is treated as a character
string whose length is bounded by the system's page size.
A record may grow at will within the limit.
Interpretation of the contents of a record is up to the user.
Each record is identified by a system assigned
.q "universal"
identifier called an RID (Record IDentifier).
The notion of RIDs allows for random access to files.
In addition, the physical ordering of records
in a file is visible through a set
of inter-record traversal operations (fetch_next, etc.),
discussed under
.q "scans"
below.
.pp
WiSS maintains a simple flat directory structure.
Each file is identified by a symbolic name.
A file is a passive entity that must be activated before it becomes
accessible to a user.
An active instance of a file is identified by a run-time ID called an
.q "open file number"
that is given to the user at the time the file
is activated.
A file may have several active instances simultaneously.
To avoid conflicts between users, concurrency control
is enforced at the file level.
In other words, when a user is updating a file, no one
else can access that file.
.pp
The routines that process sequential files can be classified
into two categories: one that treats files as a whole and the other
that manipulates individual records.

.sh 2 "File routines"
.np
wiss_createfile
(VolID, FileName, NumPages, ExtentFillFactor, PageFillFactor)
.nf
char	*FileName;
.fi
.sp 1
Create a new file with name FileName.
NumPages is the expected number of pages of the file.
The PageFillFactor (in %) is used to control filling of pages
when locality is not an issue.
Likewise, the ExtentFillFactor (in %) is used to control filling of
extents.
.np
wiss_destroyfile
(VolID, FileName, transID, lockup, cond)
.nf
char	*FileName;
int	transId;
short	lockup, cond;
.fi
.sp 1
Destroy the file with name FileName.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.sp 1
.np
int
wiss_openfile
(VolID, FileName, Mode)
.nf
char	*FileName;
.fi
.sp 1
Make available the file FileName in the requested mode.
Return an Open File number to the caller for future references to the file.
Mode can be either READ or WRITE.
WRITE permission on a file automatically implies READ permission.
\fBNOTICE\fR:  opening a file does not set any locks on the file.
.np
wiss_closefile
(OpenFileNum)
.sp 1
Release a file from further access.
.np
wiss_firstfile 
(OpenFileNum, FirstRID, trans_id, lockup, lockmode, cond)
.sp 0
.nf
RID	*FirstRID;
int	trans_id;
short	lockup;
short	lockmode;
short	cond;
.fi
.sp 1
Return (via FirstRID) the RID of the first record in the file.
trans_id identifies a transaction.  If lockup is TRUE, then the
file is locked before the file is destroyed. If cond is TRUE (as
well as lockup), conditional locking is used.  If cond is FALSE
then unconditional locking is used.
lockmode must be one of the valid lock modes: l_NL, l_IS, l_IX, 
l_S, l_SIX, l_X.
.np
wiss_nextfile 
(OpenFileNum, CurrRID, NextRID, trans_id, lockup, lockmode, cond)
.sp 0
.nf
RID	*CurrRID, *NextRID;
int	trans_id;
short	lockup;
short	lockmode;
short	cond;
.fi
.sp 1
Return (via NextRID) the RID of the record that follows CurrRID in the file.
trans_id identifies an active transaction.  If lockup is TRUE, then the
file is locked before the file is destroyed. If cond is TRUE (as
well as lockup), conditional locking is used.  If cond is FALSE
then unconditional locking is used.
lockmode must be one of the valid lock modes: l_NL, l_IS, l_IX, 
l_S, l_SIX, l_X.
.np
wiss_prevfile 
(OpenFileNum, CurrRID, PrevRID, trans_id, lockup, lockmode, cond)
.sp 0
.nf
RID	*CurrRID, *PrevRID;
int	trans_id;
short	lockup;
short	lockmode;
short	cond;
.fi
.sp 1
Return (via PrevRID) the RID of the record that precedes CurrRID in the file.
trans_id identifies a transaction.  If lockup is TRUE, then the
file is locked before the file is destroyed. If cond is TRUE (as
well as lockup), conditional locking is used.  If cond is FALSE
then unconditional locking is used.
lockmode must be one of the valid lock modes: l_NL, l_IS, l_IX, 
l_S, l_SIX, l_X.
.np
wiss_lastfile 
(OpenFileNum, LastRID, trans_id, lockup, lockmode, cond)
.sp 0
.nf
RID	*LastRID;
int	trans_id;
short	lockup;
short	lockmode;
short	cond;
.fi
.sp 1
Return (via LastRID) the RID of the last record in the file.
trans_id identifies a transaction.  If lockup is TRUE, then the
file is locked before the file is destroyed. If cond is TRUE (as
well as lockup), conditional locking is used.  If cond is FALSE
then unconditional locking is used.
lockmode must be one of the valid lock modes: l_NL, l_IS, l_IX, 
l_S, l_SIX, l_X.
.np
wiss_rename
(VolID, NewName, OldName, transId, lockup, cond)
.nf
char	*NewName, *OldName;
int     transId;
short   lockup;
short   cond;
.fi
.sp 1
Change the name of an existing file.
transId identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.br
.sh 2 "Record routines"
.np
wiss_readrecord
(OpenFileNum, RecID, RecAdr, Length, transId, lockup, lockMode, cond)
.nf
RID	*RecID;
char	*RecAdr;
int	transId;
short	lockup, lockMode, cond;
.fi
.sp 1
The record identified by RecID is read into the area pointed to by RecAdr.
Length is the expected length of the record.
The actual number of bytes read (never greater than Length) is returned.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
lockMode must be one of the valid lock modes.
.np
wiss_writerecord
(OpenFileNum, RecID, RecAdr, Length, transId, lockup, cond)
.nf
RID	*RecID;
char	*RecAdr;
int	transId;
short	lockup, cond;
.fi
.sp 1
The record identified by RecID is over-written by the new text
in the area pointed to by RecAdr.
Length is the size of the new text.
The record may grow if the new size is larger than its original size.
The actual number of bytes written is returned.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.ul
Indices (if any) must be updated separately.
.np
wiss_insertrecord
(OpenFileNum, RecAdr, Length, NearRID, NewRID, transId, lockup, cond)
.nf
char	*RecAdr;
RID	*NearRID;
RID	*NewRID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Insert a record into a file, near NearRID if possible.
RecAdr points to a user buffer where the text of the new record is stored.
The RID of the inserted record is returned through NewRID.
If NearRID is NULL, the system places the record as it pleases.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.ul
Indices (if any) must be updated separately.
.np
wiss_appendrecord
(OpenFileNum, RecAdr, Length, NewRID, transId, lockup, cond)
.nf
char	*RecAdr;
RID	*NewRID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Append a record to the end of a file.
Return the RID of the new record through NewRID.
lockup and cond are either TRUE or FALSE. 
.ul
Indices (if any) must be updated separately.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
wiss_deleterecord 
(OpenFileNum, RecID, transId, lockup, cond)
.nf
RID	*RecID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Delete the record identified by RecID from the file.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.ul
Indices (if any) must be updated separately.
.np
wiss_compare 
(OpenFileNum, RecID, Operator, Field, Value, transId, lockup, cond)
.sp 0
.nf
RID	*RecID;
KEYINFO	*Field;
KEY	*Value;
int	transId;
short	lockup, cond;
.fi
.sp 1
Compare the field (as specified by "Field") 
of record "RecID" with the given value (in "Value") 
to see if they satisfy the relation 'Operator' (NE, EQ, LT, LE, GT, GE).
This routine returns a boolean value of TRUE or FALSE.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
wiss_ridcompare 
(OpenFileNum1, RID1, F1, OpenFileNum2, RID2, F2, ReturnValue, transId, lockup, cond)
.sp 0
.nf
RID	*RID1, *RID2;
KEYINFO	*F1, *F2;
int	*ReturnValue;
int	transId;
short	lockup, cond;
.fi
.sp 1
Compare field F1 of record RID1 with field F2 of record RID2, and
return the value of their difference (ie, F1(RID1) - F2(RID2))
in ReturnValue \**.
F1 and F2 describe attributes of the fields in RID1 and RID2, respectively.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.(f
\**
For string operands, the interpretation of the return value
is similar to the that of the "strncmp" in C's library.
.)f
.sh 1 "Indices and Their Related Operations"
.(x
.sh 1 "Indices and Their Related Operations"
.)x
.pp
Indices map field values into a collection of records with
fields of that value.
An index provides an
.ul
associative
view of a file, based on the values of a specific field in each record.
(See the section on scans for more details.)
Any field, regardless of type,
is a potential candidate for building an index.
A key field must, however, have a fixed length and a fixed offset relative
to the beginning of the record structure.
(Note that this restriction on fields does not apply to records in a file
with no indices.)
A user may need to rearrange fields so that those of variable-length
appear at the end, and those with indices are in fixed locations near
the beginning.
.pp
There are two types of indices, B+-tree indices and hash indices.
A data file may have several indices built on different fields.
However, a file can have only one
.q "clustered"
B+-tree index in which the key
ordering is (almost) the same as the physical ordering of the data records.
Such a clustered index is built by creating an index
with the parameter
.q "Sorted"
set to true.
The original data file will be sorted at the time the index is built.
As a consequence, a
.q "clustered"
index must be the first to exist.
Building a clustered B+-tree index will invalidate all other existing indices.
.pp
An index on a data file is identified by a (FileName, IndexNo)
pair where
.q "FileName"
is the name of the primary data file and
IndexNo is a positive integer smaller than 1000.
It is the responsibility of the user to keep track of
index numbers for each data file.
An index is, in essence, a file with special semantics.
After an index is activated, it is identified
by an open file number just like any other file instance.
Closing an index is the same as closing a data file \**.
.(f
\**
To close an index, call wiss_closefile.
.)f
.np
wiss_createindex
(VolID, FileName, IndexNo, KeyAttr, FillFactor, Primary, Sorted, transId, lockup, cond)
.nf
char		*FileName;
KEYINFO	*KeyAttr;
int		transId;
short		lockup, cond;
.fi
.sp 1
Create a B-tree index on the named file and give it the name FileName.IndexNo.
KeyAttr is a structure of the form <key length, key offset, key type>, and
describes the field on which the index is structured.
FillFactor (in %) is a hint to the system to keep the index pages 
somewhat unfilled for future expansion.
Primary is TRUE if this is a primary index (no duplicate keys allowed).
Sorted is TRUE if the data file should be sorted on this key (i.e. if
this should be a
.q "clustered"
index).
Indices are created on single field only.
lockup and cond are either TRUE or FALSE.  
.np
wiss_dropindex
(VolID, FileName, IndexNo, transId, lockup, cond)
.nf
char	*FileName;
int	transId;
short	lockup, cond;
.fi
.sp 1
Drop the specified index built on the file FileName.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
int
wiss_openindex
(VolID, FileName, IndexNo, Mode)
.nf
char	*FileName;
.fi
.sp 1
Make the specified index file available in the requested mode
(READ or WRITE).
Returns an Open File number to the caller for future references to this index.
\fBNOTICE\fR:  opening an index does not set any locks on the index.
.np
wiss_insertindex
(OpenFileNum, Key, RecID, transId, lockup, cond)
.nf
KEY	*key;
RID	*RecID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Insert a <key, RID> pair into an index file.
This routine is intended to be used for updating indices \**.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.(f
\**
The index that is used in an index scan will be updated automatically
at the time the scan is closed.
All other indices must be updated explicitly.
.)f
.np
wiss_deleteindex
(OpenFileNum, Key, RecID, transId, lockup, cond)
.nf
KEY	*key;
RID	*RecID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Delete a <key, RID> pair from an index file.
This routine is intended to be used for updating indices.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
wiss_getindex
(filenum, SearchKey, Cursor, FirstRID, trans_id, lockup, oper, cond)
.nf
int     filenum;        /* open file number of the index */
KEY     *SearchKey;     /* lower bound for the index key */
XCURSOR *Cursor;        /* Cursor of the 1st qualified index */
RID     *FirstRID;      /* where to return the qualified RID */
int     trans_id;
short   lockup;
enum    bt_oper    oper;
short   cond;
.sp
.fi
Returns the RID of the first record (in the order of the index)
whose key is equal to the given key value specified by SearchKey.
In addition, a B-tree scan cursor is set up and returned
and returned in Cursor.
The SearchKey may be NULL. 
In this case, the first RID of the index is returned.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
oper must be one of the values BT_READ,  BT_INSERT, BT_DELETE,
BT_INSERT_DELETE defined in st_macro.h
.np
wiss_createhash
(VolID, FileName, IndexNo, KeyAttr, FillFactor, Primary, transId, lockup, cond)
.nf
char	*FileName;
KEYINFO	*KeyAttr;
int	transId;
short	lockup, cond;
.fi
.sp 1
Create a hash index on the named file and give it the name FileName.IndexNo.
KeyAttr is a structure of the form <key length, key offset, key type>, and
describes the field on which the index is structured.
FillFactor (in %) is a hint to the system to keep the index pages 
somewhat unfilled for future expansion.
Primary is TRUE if this is a primary index (no duplicate keys allowed).
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
wiss_destroyhash
(VolID, FileName, IndexNo, transId, lockup, cond)
.nf
char	*FileName;
int	transId;
short	lockup, cond;
.fi
.sp 1
Drop the specified index built on the file FileName.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
int
wiss_openhash
(VolID, FileName, IndexNo, Mode)
.nf
char	*FileName;
.fi
.sp 1
Make the specified index file available in the requested mode.
Return an Open File number to the caller for future references to this index.
\fBNOTICE\fR:  opening an index does not set any locks on the index.
.np
wiss_inserthash
(OpenFileNum, Key, RecID, transId, lockup, cond)
.nf
KEY	*key;
RID	*RecID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Insert a <key, RID> pair into an index file.
This routine is intended to be used for updating indices \**.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.(f
\**
The index that is used in an index scan will be updated automatically
at the time the scan is closed.
All other indices must be updated explicitly.
.)f
.np
wiss_deletehash
(OpenFileNum, Key, RecID, transId, lockup, cond)
.nf
KEY	*key;
RID	*RecID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Delete a <key, RID> pair from an index file.
This routine is intended to be used for updating indices.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
wiss_gethash 
(OpenFileNum, SearchKey, Xcursor, FirstRID, trans_id, lockup, cond)
.sp 0
.nf
KEY	*SearchKey;
XCURSOR	*Xcursor;
RID	*FirstRID;
int	trans_id;
short	lockup;
short	cond;
.fi
.sp 1
Return (via FirstRID) the RID of the first record 
whose key matches the search key described by <KeyType, KeyLen, KeyValue>.
The current position on the leaf level of the index is returned via Xcursor.
trans_id identifies a transaction.  If lockup is TRUE, then the
file is locked before the file is destroyed. If cond is TRUE (as
well as lockup), conditional locking is used.  If cond is FALSE
then unconditional locking is used.
.np
wiss_nexthash 
(OpenFileNum, Xcursor, NextRID, trans_id, lockup, cond)
.sp 0
.nf
XCURSOR	*Xcursor;
RID	*NextRID;
int	trans_id;
short	lockup;
short	cond;
.fi
.sp 1
Return (via NextRID) the RID of the next record having the same key
as the one under the cursor.
The cursor is updated to point to the location of NextRID in the leaf page.
trans_id identifies an active transaction.  If lockup is TRUE, then the
file is locked before the file is destroyed. If cond is TRUE (as
well as lockup), conditional locking is used.  If cond is FALSE
then unconditional locking is used.

.sh 1 "Long Data Items"
.(x
.sh 1 "Long Data Items"
.)x
.pp
Long data items provide a facility for storing variable
length data objects that may grow to virtually unlimited size.
They are regarded as simple streams of bytes.
A long data item is identified by the RID of its directory \**.
.(f
\**
In some respects, a long data item can be viewed as an collection
of records within a file that constitute an integrated super-long field.
Pointers to those records,
together with other information, are maintained
in another special record, which we refer to as the directory of the item.
A long data item is intended to be used as a field within some record.
.)f
Operations are provided for creating and destroying long data items,
and for reading and changing their contents.
More details will be given in the section on scans.

.np
wiss_createlong
(OpenFileNum, DirRID, transId, lockup, cond)
.nf
RID	*DirRID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Create a long data item, and return the RID of its directory.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
wiss_destroylong
(OpenFileNum, DirRID, transId, lockup, cond)
.nf
RID	*DirRID;
int	transId;
short	lockup, cond;
.fi
.sp 1
Destroy a long data item whose directory is identified by DirRID.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.br
.sh 1 "Scans"
.(x
.sh 1 "Scans"
.)x
.pp
A
.q "scan"
can be thought of as a process for going through a list
of data objects in a certain well-defined order.
For the purposes of discussion, we introduce the notion of
.ul
cursor\c
s for scans.
Each scan has a cursor that points to the
object currently under inspection.
Cursors can be memorized (not necessarily in the same scan)
as back-up points
that allow a user to start over at any position in a file.
.pp
A scan introduces an order on the records of a
file.
The user is not forced to access records in that order.
The order is useful, however, because it establishes
.ul
relative
positions; it allows us to refer to the
.q "next"
record, the
.q "previous"
record, the
.q "last"
record, etc.
.pp
WiSS supports several types of scans on sequential files and on long data items.
Scans on sequential files allow a user to go through a data file
either in sequential (physical) order or in order of
the key values of an existing index.
A simple but powerful filtering mechanism is built into the scan interface.
A user can specify a search criterion at the time a scan is opened.
Only records that satisfy the criterion will be included in the scan.
For a B-tree indexed scan, the user can also specify upper and lower bounds
on the key values of the indexed field.  
This allows a scan to start where the
user wants without performing wasteful searches.
For a hash index scan, the user specify the search key.
Only records with a matching key value will be retuned.
Since indices apply only to sequential files, sequential order is the 
only available option for scans on a long data item.
.pp
The boolean expressions in the routines below consist of a linked list
of boolean terms,
semantically AND-ed together.
Each boolean term is a relational expression that contains two
operands: a constant value and a template (field descriptor).
When evaluating a boolean expression to determine whether a particular
record should be included in a scan,
the contents of specified fields are extracted from the record
and compared with the constant value.
The exact format of a boolean expression is given in Appendix A.

.sh 2 "Initiating and terminating scans"
.pp
In WiSS, scans are performed on
active file instances rather than on the files themselves.
An indexed scan of a sequential file involves two instances: one
for the index and one for the sequential file.
Other scans involve only one file instance.
Because active instances are used,
the file(s) involved in a scan must be opened before the scan starts,
and must remain open until it finishes.
Closing a file that has outstanding scans
will automatically invalidate those scans.
.pp
Several scans may be active simultaneously on a single file instance \**.
.(f
\**
Recall that a file may have several file instances active.
Each of these file instances, in term, may have several scans active.
.)f
Scans may be read-only, or they may be permitted to modify the data
involved.
An update scan is allowed to coexist with other read-only scans
provided that they all belong to the same user.
Under the current implementation,
updates take effect immediately in scans of long data items and in
sequential scans of sequential files.
For index scans, updates are visible (i.e. they
.ul
commit)
only after the scan is closed.
.np
int
wiss_openfilescan
(OpenFileNum, BooleanExpr, transId, lockup, lockMode, cond)
.nf
BOOLEXP	*BooleanExpr;
int	transId;
short	lockup, cond, lockMode;
.fi
.sp 1
Return the ID of a new sequential scan over the file opened with
OpenFileNum.
BooleanExpr is a linked list of simple boolean terms.
Only those records that satisfy all of the boolean conditions will be
fetched by this scan.
A null boolean expression evaluates to TRUE.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.  
lockMode specifies the lock mode for the file.
.np
int
wiss_openindexscan
(OpenFileNum, IndexFileNum, IndexKey, LB, UB, BooleanExpr, transId, lockup, lockMode, cond)
.nf
KEYINFO	*IndexKey;	/* key attribute of the index */
KEY	*LB;		/* lower bound value */
KEY	*UB;		/* upper bound value */
BOOLEXP *BooleanExpr;	/* search predicate */
int	transId;
short	lockup, cond, lockMode;
.fi
.sp 1
Return the ID of a new index scan over the index identified by IndexFileNum.
Only those records that fall between the lower and upper bounds and
that satisfy all terms of the Boolean expression will be included in
the scan.
IndexKey describes the key on which the index is built.
For an update index scan, WRITE permissions are required on both
the file and the index.
A null boolean expression evaluates to TRUE.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
lockMode specifies the lock mode for both the file and the index.
If lockMode = IX (IS), then the file and index are locked in IX (IS) mode
and the pages of the file and the index are locked in X (S) mode.
If lockMode = X (S), then the file and index are locked in X (S) mode
and the pages are accessed without setting any locks.
.np
int
wiss_openhashscan
(OpenFileNum, IndexFileNum, IndexKey, SearchKey, BooleanExpr, transId, lockup, lockMode, cond)
.nf
KEYINFO	*IndexKey;	/* key attribute of the index */
KEY	*SearchKey;	/* which key to search */
BOOLEXP *BooleanExpr;	/* search predicate */
int	transId;
short	lockup, cond, lockMode;
.fi
.sp 1
Return the ID of a new index scan over the index identified by IndexFileNum.
Only those records that has the specified key value, and
that satisfies all terms of the Boolean expression will be included in
the scan.
IndexKey describes the key on which the index is built.
For an update index scan, WRITE permissions are required on both
the file and the index.
A null boolean expression evaluates to TRUE.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
lockMode specifies the lock mode for both the file and the index.
If lockMode = IX (IS), then the file and index are locked in IX (IS) mode
and the pages of the file and the index are locked in X (S) mode.
If lockMode = X (S), then the file and index are locked in X (S) mode
and the pages are accessed without setting any locks.
.np
int
wiss_openlongscan
(OpenFileNum, DirRID, transId, lockup, lockMode, cond)
.nf
RID	*DirRID;
int	transId;
short	lockup, cond, lockMode;
.fi
.sp 1
Return the ID of a new scan over the long data item contained in the
specified file. The directory of the long data item
is stored in the record identified by DirRID.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
lockMode specifies the lock mode for long date item.
.np
wiss_closescan
(ScanID)
.sp 1
Deactivate the scan identified by ScanID.
For an index scan, the updates are committed here.

.sh 2 "Movement of scan cursors"
.(x
.sh 2 "Movement of scan cursors"
.)x
.pp
The routines in this group deal with cursor movements inside a file
or a long data item.
These routines enable a user to go through a file
either by physical ordering or by key ordering.
In addition, a user can change the path of a scan by
moving the cursor to any valid position within a data file or a long data item.
.np
wiss_fetchfirst
(ScanID, FirstRID)
.nf
RID	*FirstRID;
.fi
.sp 1
Return via FirstRID the RID of the first record
that satisfies the search criterion of the scan.
.ul
For sequential and index scans only.
.np
wiss_fetchnext
(ScanID, NextRID)
.nf
RID	*NextRID;
.fi
.sp 1
Return via NextRID the RID of the next record
that satisfies the search criterion of the scan.
.ul
For sequential and index scans only.
.np
wiss_fetchprev
(ScanID, PrevRID)
.nf
RID	*PrevRID;
.fi
.sp 1
Return via PrevRID the RID of the previous record
that satisfies the search criterion of the scan.
.ul
For sequential and index scans only.
.np
wiss_fetchlast
(ScanID, LastRID)
.nf
RID	*LastRID;
.fi
.sp 1
Return via LastRID the RID of the last record
that satisfies the search criterion of the scan.
.ul
For sequential and index scans only.
.np
wiss_setscan
(ScanID, RecID)
.nf
RID	*RecID;
.fi
.sp 1
Set the cursor of a sequential scan to RecID.
It is an error if RecID does not belong to the file associated with this scan.
Currently, this routine does not check for such errors.
Any attempt to use an illegal RID will, however, be caught.
.ul
For sequential scans only.
.np
wiss_geticursor
(ScanID, Xcursor)
.nf
XCURSOR	*Xcursor;
.fi
.sp 1
Return the cursor of an index scan.
This index cursor can be used later to reset the cursor of 
the index scan (by calling wiss_setiscan).
.ul
For index scans only.
.np
wiss_setiscan 
(ScanID, Xcursor)
.nf
XCURSOR	*Xcursor;
.fi
.sp 1
Re-position the cursor of an index scan to Xcursor.
.ul
For index scans only.
.np
wiss_setcursor
(ScanID, Offset, Relocation)
.sp 1
Set the cursor for the long item scan to the specified byte Offset and
Relocation.
Offset is a 4-byte integer.
Relocation is 0 for positioning relative to the first byte in the item,
1 for positioning relative to the current cursor position,
and 2 for positioning relative to the last byte in the item.
.ul
For scans on long data items only.

.sh 2 "Accessing records and long data items in a scan"
.(x
.sh 2 "Accessing records and long data items in a scan"
.)x
.pp
The routines in this group are those that actually
access the data objects (records or long data items).
Operations are performed relative to the current cursor position.
For example, a
.q "read"
operation for a sequential file scan
returns the contents of the record currently under the scan cursor.

.np
wiss_readscan
(ScanID, RecAdr, Length)
.nf
char	*RecAdr;
.fi
.sp 1
Read the record at the current cursor position.
Length is the expected length of the record.
Return the actual number of bytes read.
.ul
For sequential and index scans only.
.np
wiss_readlong
(ScanID, RecAdr, Length)
.nf
char	*RecAdr;
.fi
.sp 1
Read Length bytes into RecAdr from a long data item.
Return the actual number of bytes read.
.ul
For scans on long data items only.
.np
wiss_insertscan
(ScanID, RecAdr, Length, NewRID)
.nf
char	*RecAdr;
RID	*NewRID;
.fi
.sp 1
Insert a record near the cursor of a scan and
return (via NewRID) the RID of the new record.
The text of the new record, of length Len, is in a user
buffer pointed to by RecAdr.
.ul
For sequential and index scans only.
.np
wiss_insertlong
(ScanID, After, RecAdr, Length)
.nf
char	*RecAdr;
.fi
.sp 1
Insert a string of bytes into a long data item.
If After is TRUE, insert after the current byte in the scan; otherwise
insert before the current byte.
.ul
For scans on long data items only.
.np
wiss_updatescan
(ScanID, RecAdr, Length)
.nf
char	*RecAdr;
.fi
.sp
Update the record in the file we are currently scanning.
The new record is in a buffer area in user space,
pointed to by RecAdr.
.ul
For sequential and index scans only.
.np
wiss_updatelong 
(ScanID, RecAdr, Length)
.nf
char	*RecAdr;
.fi
.sp
Replace Length bytes of a long data item, starting from the cursor
position, with the new text in the buffer pointed to by RecAdr.
Return the actual number of bytes written.
.ul
For scans on long data items only.
.np
wiss_deletescan
(ScanID)
.nf
.fi
.sp 1
Delete the record under the scan cursor.
.ul
For sequential and index scans only.
.np
wiss_deletelong
(ScanID, Length)
.sp 1
Remove Length bytes, beginning at the cursor, from the long data item
being scanned.
.ul
For scans on long data items only.

.sh 1 "Sort Utilities"
.(x
.sh 1 "Sort Utilities"
.)x
.pp
WiSS provides two sort utilities.
The two are much alike, except that
one sorts in place (at least conceptually),
and the other places the sorted records into a new file.
Given a data file, either sort routine re-arranges
records on the basis of some specified key field.
Sorting a file
invalidates all its RIDs and hence all its existing indices.

.np
wiss_sort
(VolID, FileName, KeyAttr, Suffix, transId, lockup, cond)
.nf
char	*FileName;
KEYINFO	*KeyAttr;
int 	transId;
short	lockup, cond;
.fi
.sp 1
Sort a file on the given field.
KeyAttr describes the field on which the file is to be sorted.
Suffix is a number that is used to generate temporary file names.
(Suffix should not be the same as the index number of 
any existing index on the file.)
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.np
wiss_sortinto
(VolID, SourceFile, ResultFile, KeyAttr, Suffix, transId, lockup, cond)
.nf
char	*SourceFile;
char	*ResultFile;
KEYINFO	*KeyAttr;
int 	transId;
short	lockup, cond;
.fi
.sp 1
Sort the source file on the field given, and put the sorted result
into another file.
KeyAttr describes
the field on which the file is to be sorted.
Suffix is a number that is used to
generate temporary file names.
(Suffix should not be the same as the index number of 
any existing index on the file.)
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.br
.sh 1 "Routines for Advanced Users Only"
.(x
.sh 1 "Routines for Advanced Users Only"
.)x
.pp
The following routines avoid the copy-based interface and allow
the programmer to directly operate on records in the buffer pool.
They should only be used on last resort as they are very, very
prone to problems.
.sp
.np
int wiss_getrecord 
(filenum, ridptr, pageptr, recptr, tranid, lockup, mode, cond)
.nf
int filenum;
RID *ridptr;  
DATAPAGE **dp;
char **recptr;
int transid;
LOCKTYPE mode;
short lockup, cond;
.sp
.fi
Reads the record identified by ridptr into the buffer pool.
The address of the data portion of the record is returned in recptr.  
The address of the page containing the record is returned in dp.  
The size of the record in bytes is returned as a result parameter.
The page is left pinned in the buffer pool.  It is the user's
responsibility to make sure that the page gets unpinned eventually.
.sp
NOTE!!.  st_readrecord differs from r_getrecord in that instead
of returning a pointer to the wiss record, st_getrecord() returns
a pointer to the data portion of the wiss record along with
the length of the record.
.br
.np
int wiss_expandrecord 
(filenum, ridptr, expandamt, transid, lockup, cond)\fR
.nf
int filenum;
RID *ridptr;  
int expandamt;
int	transid;
short lockup, cond;
.sp
.fi
Extends the record identified by ridptr by expandamt bytes.
\fBNOTE!!!\fR This may cause the record to be moved from its current location
to a new data page.  Thus, before calling st_expandrecord(),
one must make sure that the page containing the record is NOT pinned.
.sp
\fBWARNING!!!\fR Any other pointers to records on the same page as
the expanded record (or, in the case where has to be moved, to
its new page),  may no longer be valid after this call is executed.
Basically the only safe way to use this call is with no pointers
into the buffer pool.
.br
.np
int bf_unpin
(dp, dirty)
.nf
DATAPAGE *dp;
int	dirty;
.sp
.fi
unpins the buffer frame identified by dp. If dirty == TRUE, the
buffer is marked as dirty.
.sp
\fBIMPORTANT:\fR
.sp
The following sequence \fBmust be used\fR to safely expand a record:
.(b
e = bf_unpin (dp, dirty);
CHECKERROR(e);
e= wiss_expandrecord (filenum, ridptr, expandamt, transid, lockup, cond)
CHECKERROR(e);
e= wiss_getrecord (filenum, ridptr, pageptr, recptr, tranid, lockup, cond)
CHECKERROR(e);
.)b
.fi
.np
st_firstscan 
(OpenFileNum, FirstRID, recPtr, recLen, trans_id, lockup, lockmode, cond)
.sp 0
.nf
RID	*FirstRID;
char	**recPtr;
int	*recLen;
int	trans_id;
short	lockup;
short	lockmode;
short	cond;
.fi
.sp 1
For the first record in the file this routine returns:
its RID via FirstRid, a pointer to the record in the buffer pool 
(via recPtr),  and its length (via recLen).
\fBThe page containing the record is left pinned in the buffer pool.\fR
trans_id identifies a transaction.  If lockup is TRUE, then the
file is locked before the file is destroyed. If cond is TRUE (as
well as lockup), conditional locking is used.  If cond is FALSE
then unconditional locking is used.
lockmode must be one of the valid lock modes: l_NL, l_IS, l_IX, 
l_S, l_SIX, l_X.
.np
st_nextscan 
(OpenFileNum, CurrRID, NextRID, recPtr, recLen, trans_id, lockup, lockmode, cond)
.sp 0
.nf
RID	*CurrRID, *NextRID;
char	**recPtr;
int	*recLen;
int	trans_id;
short	lockup;
short	lockmode;
short	cond;
.fi
.sp 1
Return for the record that follows CurrRid in the file,
its RID (via NextRid), a pointer to the record in the buffer pool 
(via recPtr),  and its length (via recLen).
\fBThe page containing the record is left pinned in the buffer pool until
all the records on that page have been scanned at which point
the page is automatically unpinned.\fR
trans_id identifies an active transaction.  
If lockup is TRUE, then the
file is locked before the file is destroyed. If cond is TRUE (as
well as lockup), conditional locking is used.  If cond is FALSE
then unconditional locking is used.
lockmode must be one of the valid lock modes: l_NL, l_IS, l_IX, 
l_S, l_SIX, l_X.
.np
wiss_appendfile
(OpenFileNum, RecAdr, Length, NewRID, transId, lockup, cond)
.nf
char	*RecAdr;
RID	*NewRID;
int	transId;
short	lockup, cond;
.fi
.sp 1
This is a special version of wiss_appendrecord that
appends a record to the end of a file.  The difference
between wiss_appendfile and wiss_appendrecord is that wiss_appendfile
keeps the last page of the file pinned in the buffer pool
between calls. After the last page has been filled, the page
is automatically unpinned. Returns the RID of the new record through NewRID.
lockup and cond are either TRUE or FALSE. 
.ul
Indices (if any) must be updated separately.
transID identifies an active transaction.  lockup is either
TRUE or FALSE indicating whether locking is to be performed.
If lockup is true, then cond indicates whether conditional
locking (cond == TRUE) or unconditional locking (cond == FALSE)
is to be used.
.br
.lp
.sh 1 "Statistics on Files and Indices"
.(x
.sh 1 "Statistics on Files and Indices"
.)x
.pp
The routines in this section are provided for getting statistics
about sequential files and indices.
These statistics can be used, for example, by an access plan optimizer.

.np
wiss_keycard
(VolID, FileName, IndexNo)
.nf
char	*FileName;
.fi
.sp 1
Return the number of distinct keys in a b-tree index.
.np
wiss_recordcard
(VolID, FileName)
.nf
char	*FileName;
.fi
.sp 1
Return the number of records in the file.
.np
wiss_indexpages
(VolID, FileName, IndexNo)
.nf
char	*FileName;
.fi
.sp 1
Return the number of pages in a b-tree index.
.np
wiss_filepages
(VolID, FileName)
.nf
char	*FileName;
.fi
.sp 1
Return the number of pages in the file.
.br
.np
int wiss_hashcard
(VolID, FileName, IndexNo)
.nf
char *Filename;  int VolID, IndexNo;
.fi
.sp 1
Return the number of distinct keys in a hash index
.br
.np
wiss_hashpages 
(VolID, FileName)
.nf
char *Filename;  int VolID;
.fi
.sp 1
Return the number of pages in a hash index.
.br
.sh 1 "Miscellaneous Operations"
.(x
.sh 1 "Miscellaneous Operations"
.)x
.pp
There are two routines, wiss_init and wiss_final,
needed to bring the system up or down, respectively.
Wiss_init initializes all the in-core system tables; while wiss_final
flushes all the information cached in main memory to disk.
In the face of fatal errors, wiss_final tries its best to keep
the system in a consistent state.
.pp
Devices may be mounted after system initialization.
The ID of the volume mounted on a device is returned by wiss_mount.
This ID is required for future access to the data on the volume.
A volume that is no longer needed for access
can be deactivated by dismounting the device the volume is attached to.
.pp
Finally, there are two routines provided to aid debugging.
They provide explanations of WiSS error codes.
The messages the routines produce may not be very useful to a user,
but they give a clue as to where the error originated or was detected.
.br
.np
wiss_init 
()
.sp 1
Initialize WiSS for system startup.
.np
wiss_final
()
.sp 1
Finalize WiSS for system shutdown.
This include flushing out all the dirty pages in the buffer pool
and other clean-up procedures.
.np
int
wiss_mount
(DeviceName)
.fi
char	*DeviceName;
.fi
.sp 1
Mount the named device, and return the Volume ID of the volume on the device.
.np
wiss_dismount
(DeviceName)
.nf
char	*DeviceName;
.fi
.sp 1
Dismount the named device.
.np
wiss_create_volume
(DeviceName, Title, Volid, NumExts, ExtSize)
.nf
char	*DeviceName;
char	*Title;
int	Volid;
int	NumExts;
int	Extsize;
.fi
.sp 1
Creates a volume named Title with volume identifier Volid 
in the Unix file named DeviceName.  The number of extents
in the volume is specified by NumExts and Extsize
specifies the size of each extent in pages.
If the volume cannot be created,  the error message e0MOUNTFAILED
is returned.
If a volume with the same name is already mounted it is first dismounted.
.np
wiss_destroy_volume
(DeviceName)
.nf
char	*DeviceName;
.fi
.sp 1
Destroys the volume named DeviceName.  If the volume is already
mounted, the error e0VOLMOUNTED is returned and the Unix file
containing the volume is not unlinked.  If the unlink fails
the call will return a unix error code.
.np
wiss_error
(Message, ErrorCode)
.nf
char	*Message;
.fi
.sp 1
Explain the meaning of an error code.
Message is any additional information the caller wishes to display
when the error occurs.
.np
wiss_fatalerror
(Message, ErrorCode)
.nf
char	*Message;
.fi
.sp 1
Explain the meaning of an error code and terminate.
Message is any additional information the caller wishes to
display when the error occurs.

.sh 1 "Interfacing with WiSS"
.(x
.sh 1 "Interfacing with WiSS"
.)x
.pp
To interface with WiSS, a user program needs to include
three header files, \fIwiss.h\fR and \fIwiss_r.h\fR (from \fIwiss\fR).
The first file, \fIwiss.h\fR, contains all the data structure definitions
that a WiSS user needs;
the second file, \fIwiss_r.h\fR, maps official WiSS routine names
into their real names in the system.
The third file, \fIlockquiz.h\fR, defines the various lock modes
that the system understands (i.e., l_S, l_IS, etc.)
After a user program is compiled, the final step is to
link the user's object code with \fIbin/wiss.o\fR, which contains the
entire object of WiSS.
.bp
.ce
Appendix A - Data Types
.(b

/* machine-dependent data types */
typedef int	FOUR;
typedef short	TWO;
typedef char	ONE;

typedef struct 	/* record identifier */
{	FOUR	Rpage;
	TWO	Rslot;
	TWO	Rvolid;
} RID;

/* data types supported */
typedef	enum	data_type	{TINTEGER, TLONG, TSHORT,
				 TFLOAT, TDOUBLE, TSTRING, TBITS};

/* relational operators used in Boolean expressions */
typedef	enum	rel_op		{EQ, NE, LT, LE, GT, GE};

typedef	struct	/* field descriptor */
{	TWO	offset;		/* field offset into record */
	TWO	length;		/* field length */
	enum data_type 	type;	/* data type of this field */
} FIELDDESC;

typedef	struct	/* data descriptor */
{	enum data_type 	type;		/* type of the data */
	TWO		length;		/* length of the data */
	char		value[MAXFIELD];/* value of the data */
} DATADESC;

typedef	struct  /* boolean term */
{
   enum rel_op	operator;	/* relational operator */
   FIELDDESC	fielddesc;	/* field to be compared with */
   int 		next;		/* pointer (offset) to next term */
   char	value[MAXFIELD];	/* value to be compared with */
} BOOLEXP;

typedef	struct /* cursor to an index leaf */
{
	PID	pageid;		/* page on which the index is on */
	TWO	slotnum;	/* slot # of the key of this index */
	TWO	offset;		/* offset into the RID list */
} XCURSOR;

typedef FIELDDESC	KEYINFO;
typedef	DATADESC	KEY;
.)b
.sp 0

