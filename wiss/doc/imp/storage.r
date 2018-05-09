.ig

last edited by
chou (4/1/84)
..
.sh 2 "Sequential Data Organization"
.(x
.sh 2 "Sequential Data Organization"
.)x
.pp
Sequential organization may be used for creating files where
sorted order of records is important.
A sequential file may have random, clustered, or sorted order.
Random order implies that the file is not sorted on any attribute and that
insertions may be done anywhere.
Sorted order means the file is sorted on some attribute and insertions require
resorting if the record cannot be put in the exact spot.
Clustered order is similar to sorted order except that exact order is not
required: a record may be put near its correct spot.
The main difference here is that insertions do not require resorting.

.sh 3 "Data pages and Records"
.(x
.sh 3 "Data pages and Records"
.)x
.pp
We view records as black boxes.
If a particular record is a tuple in a relational database, it may
well have its own header indicating the number of fields, lengths and/or
offsets of fields, and other information.
If a record is a text stream, it might not have any such headers at all.
At any rate, the presence or absence of headers not mentioned here is
assumed to be part of the data presented to the storage system.
.pp
Each record in a WiSS database has a unique Record Identifier (RID).
The RID associated with a record is permanent, and
indicates where the data is actually stored.
The RID has the format
.sp
.ce
< Volume ID, Page Address on Volume, Slot on Page >
.sp
The "Volume ID" field identifies an individual storage medium, rather
than the device on which the medium is mounted.
This provides for the situations where a medium might be mounted on
one of several devices; or one of several media is mounted on a
single device (such as an archive).
With this mechanism, records may be given names which are
unique for the life time of the record.
.pp
All data pages (see Appendix \n+(AP)
consist of some fixed number of bytes allocated 
contiguously on secondary storage.
Pages in a file are doubly-linked to provide a nominal "sequential" scan;
the pointers to do this are stored at the end of the page.
The full Page ID of the current page is (redundantly) stored on the page to
facilitate error checking;
and the File ID of the file to which this page belongs is stored on-page to
provide easy verification that a page does indeed belong to a given file.
The number of slots, or RIDs valid on this page is stored, and space for
that many slots is allocated.
The number of slots on a page can grow and shrink as records are added 
to or removed from the page.
However, since records may be removed in any order, and since the RID of 
a record must not change, there may be unused slots within a page 
which nevertheless may not be returned to the free area.
.pp
The record header for the storage Level is very simple:
.sp
.ce 
< Record Type, Record Kind, Length of Data >
.sp
.ce 2
Record Type : Moved, Not Moved, New Home,
Record Kind : Normal, Slice, Crumb.
.pp
If a record grows to be too big for the page it resides on, it may be
moved to another location, and a marker left in its place.
The marker will be a record indicating that the data has been moved;
the data for the marker record will be the RID of the new location.
If a moved record outgrows its new home, there is no need to leave
another marker behind: the only reference to its current physical location
is in the original marker, so that may be updated.
.pp
Because a record is globally referenced only by its RID, it is freely
relocatable within its data page: only its corresponding slot (which the
RID actually addresses) need be updated.
In particular, when another record in the page grows or shrinks,
the entire page may be shifted, and the free space pointer updated.
In addition, the records may appear in any order in the page, not
necessarily in their RID order.

.sh 3 "Decisions"
.(x
.sh 3 "Decisions"
.)x
.pp
The lengths of most of the address structures used by WiSS
depend on a very few parameters.
.sp 1
.in 4
.ti 2
- Using a single byte for addressing allows only 256 locations.
A two-byte address allows 65,536 (64K) locations, which is certainly
sufficient for a page.
In fact, it is probably excessive--let a page be 4K bytes long.
.sp
.ti 2
- The capacity of storage devices keeps growing.
A four-byte page address will allow for 4G pages on a device; at 4K bytes
per page, this translates to 16T (approximately 16 trillion) bytes on a device.
.sp
.ti 2
- Volume ID's should probably be unique across a wide range of systems,
especially if we wish to share volumes;
if we allocate 2 bytes (16 bits) for this, we have 64K distinct
volume identifiers.
.in 0
.sp 1
These three parameters yield an 8-byte RID.
It takes only two bytes to address into a page.
This suffices for the free space pointer, the slots (pointers),
and the slot count at the end of a page.

.sh 3 "Routines"
.(x
.sh 3 "Routines"
.)x
.sp
.de RR
.np
\\$1  (in \\$2.c)
.sp 0
..
.RR "r_addrec" "r_addrec"
Add a record to a page and return the RID of the newly created record
if there is room; otherwise do nothing and return an error code.
.RR "r_dropout" "r_dropout"
Unlink a data page from its predecessor and successor.
This routine updates the first page and/or the last page of the 
file as necessary.
.RR "r_dumppage" "r_dumppage"
Print the control information of a page, and display the records as
ASCII texts. 
This is intended for debugging purposes.
.RR "r_getrecord" "r_getrecord"
Convert a RID into a memory address.
.RR "r_hookup" "r_hookup"
Link a data page between two others.
This routine updates the first page and/or the last page of the 
file as necessary.
.RR "r_slide" "r_slide"
Create, destroy, or change sizes of records on a data page by
moving the data area around and/or updating the on-page pointers in the
slots.
The data of the modified record is of questionable use on exit from this
routine, but the length field of the header is correct.
.in 0

