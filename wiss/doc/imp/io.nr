.ig
last edited by
chou (7/12/84)
..
.sh 1 "The Input/Output Level"
.(x
.sh 1 "The Input/Output Level"
.)x
.pp
The storage model of WiSS has volumes (disk packs, tapes, etc.) mounted
on devices (disk drives, tape drives, etc.).
Higher levels of WiSS don't care what the device is, requiring only that it be
random-access
(for the purposes of this paper, however, disk will be used for examples).
Each volume has a unique volume identifier, thus allowing a volume to be
mounted on different devices from time to time.
.pp
The initial version of level 0 is based on the UNIX file system, for
expediency.
In this implementation, a volume is represented by a UNIX file.
It is artificially divided into pages and extents (contiguous groups of
pages).
Each volume has a unique volume ID, used by upper levels; but the
physical device number stored in level 0 is actually the number returned by
the UNIX routine open(2).
WiSS files are allocated as pieces of the UNIX file (WiSS volume).
.pp
The implementation of level 0 on top of a raw device is similar.
There is a 'raw' interface on UNIX that provides for direct transmission
of blocks between a disk and a user's buffer.
The blocks I/O's of a raw file bypass the disk address translation
and block buffering of the UNIX file system.
To deal with a 'raw device', we treat it as a UNIX file, which has a
system-defined file name \**. 
The system calls (read, write, etc.) are analogous to those for ordinary
UNIX files, with some restrictions on the number of bytes that can be
transferred at each call.
.(f
\**
For a disk interfaced by the MASSBUS, the name of the raw files on it
is referred to as "/dev/rhp[0-7][a-h]".
.)f
.pp
To avoid confusion below,
we will ignore the fact that level 0 is actually implemented on top of UNIX,
and terminology will be that of a genuine raw-device system:
"file" refers to a WiSS file, never to a UNIX file;
"volume" refers to a WiSS volume (which is actually a UNIX file).

.sh 2 "Physical Organization"
.(x
.sh 2 "Physical Organization"
.)x
.pp
Items in a file are considered to be related: if one item in a file is
accessed, it is likely that other items in the file will also be accessed.
This concept is known as "locality of reference".
To reduce access time between items in a file, storage is allocated in
contiguous groups called
.ul
extents.
The main qualification of an extent is that accesses within a single extent
are much faster than accesses between different extents.
.pp
Allocation of pages for a file is kept clustered by allocating pages 
within an extent.
If there are no pages deemed suitable in any of a file's extents, a new
extent is appropriated for the file, and a page allocated from the new extent.
A file may be made up of one or more extents.
The descriptor for a file contains a list of those extents.
Also in the file descriptor is an extent fill factor: when an extent
exceeds its fill factor, the only new pages allocated in that extent
will be for "near page X" requests.
.pp
A volume header is located at the start of a disk (cylinder 0).
It contains information describing the volume, including
a general description of the volume, an extent allocation map,
a set of page allocation maps (one for each extent), 
and a list of file descriptors.
For the ease of implementation and the consideration for crash recovery,
a volume header is organized into four areas,
each stored on a distinct set of blocks (physical pages).
.pp
The first area (see Appendix \n+(AP),
contains identifying information for the volume:
volume ID (a number uniquely identifying this volume), and
volume Title (a string describing the volume).
In addition, this area contains information regarding the
physical structure of the device:
the size of an extent (in pages);
the number of extents on the device;
the number of free extents on the device;
the maximum number of files allowed on the device;
a count of existing files;
a bit map of extent allocation (free/used);
and a remapping list for bad pages on the device.
This area also has the page address for the root page of
the level 2 directory structure, which maps file
names into file numbers.
It may be worth noting that the file directory is always the first file 
(file 0) on a volume.
.pp
The second area of a volume header contains a set of page maps, 
one for each extent on the device.
For an extent in use, its corresponding page map keep tracks of
the allocation status (free or used) of each page in the extent.
A page map is aligned to byte (8-bit) boundary.
To simplify manipulation of bit maps, the page maps are grouped
into blocks so that no page maps cross block boundary.
.pp
The third area contains descriptors for all files on the volume.
A linked list is maintained for each file to keep track of
the extents allocated to the file.
To simplify management of extent lists, the links
for chaining up extents are organized as an array of extent numbers,
stored in the fourth area.
The content of an array element points to the next extent of the same file.
A field in a file descriptor points to the start of the file's extent list.
Another piece of information contained in a file descriptor is
the "extent fill factor" for the file as described previously in this section.
.sh 2 "Decisions"
.(x
.sh 2 "Decisions"
.)x
.pp
The first cylinder (number 0) of a WiSS volume will most likely contain
bootstrap information and the like, due to operating system bootstrap
conventions.
In an extent-based system such as WiSS, this makes cylinder 0 unattractive for
normal disk allocation, as part of this area will always be reserved.
Thus, level 0 uses this cylinder for WiSS system overhead.
The volume header, including such items as volume ID, bit maps of free
extents and pages, and bad page remap list, is stored on cylinder 0.
The list of files on this volume, stored in the file directory (and
maintained by the Storage Level of WiSS, level 2) also resides in this area.
Even with all this information, however, cylinder 0 is likely to be
fairly empty (see below).
.pp
For reasons of expediency, we decided that
each extent may belong to only one file.
The wisdom of this decision is debatable, and open to revision in the future.
There are at least two major drawbacks to the above decision:
.in 4
.ti 2
- Small files which are not expected to grow are very space-inefficient
if extents are large.
.ti 2
- It is not possible to mix different record types (i.e., different files) in
the same physical area; this feature may be desired for some applications
(for example, to speed JOIN operations).
.in 0
.pp
Some functionality which might reasonably be argued to belong to level 0
has been deliberately left out or moved to another level
(examples include linking the pages of a file together,
keeping track of the first and last pages of a file, and
managing the file directory).
The design goal behind such decisions was to keep level 0 as ignorant of
the higher-level details of WiSS as possible.
The result is that level 0 performs input and output on a page level, but knows
virtually nothing about files, and nothing at all about records.

.sh 2 "Data Structures"
.(x
.sh 2 "Data Structures"
.)x
.pp
The main data structure shared by level 0 routines is an array named "VolDev".
This table keeps track of the volumes which have been made known to the
system and brought on-line ("mounted" volumes).
When a volume is mounted, its device name is given; level 0 reads the first
block of the volume header for this device and stores into VolDev
.in 4
.ti 2
- the name of the device
.ti 2
- the volume ID of the volume on the device
.ti 2
- the physical device address for future reference
.ti 2
- the addresses of newly-allocated buffers to hold the various areas
of the volume header
.in 0
.pp
All references to a volume are made by its volume ID; this requires that
the level 0 routines perform a search in VolDev.
It may be worth considering a change whereby an on-line volume number is
supplied to the user for subsequent reference to a volume.
This would be analogous to the open-file number of level 2 (and of UNIX
files).

.sh 2 "Routines"
.(x
.sh 2 "Routines"
.)x
.de RR
.np
\\$1  (in \\$2.c)
.sp 0
..
.RR "IO_AllocExtents" "IOextent"
Allocate a given number of free extents.
This consists of locating available extents and
clearing their places in the bit map of free extents.
.RR "IO_AllocFile" "IOfiman"
Allocate a new file in the volume header with a given number of extents;
create a File ID (volume ID and File Number) corresponding to this file
and return it to the caller.
.RR "IO_AllocPageInExt()" "IOextent"
Allocates pages in a given extent.
.RR "IO_DisMount" "IOmount"
Turn off-line (in the UNIX-based system, close(2)) the device whose index
into VolDev we are given.
Remove the entry from VolDev.
.RR "IO_FlushHeader" "IOmount"
Force to be written to disk all volume header blocks for a specific volume.
.RR "IO_FreeExtents" "IOextent"
Say that the specified extents are no longer in use.
.RR "IO_FreeFile()" "IOfiman"
Removes a file and releases the extents it owns.
.RR "IO_FreePageInExt()" "IOextent"
Frees pages in a given extent.
.RR "IO_Mount" "IOmount"
Bring a specified device on line (in the UNIX fiction, open(2)), and
fill in the appropriate information in VolDev.
.RR "IO_ReadPage" "IOreadwrite"
Read a page from a physical device, either synchronously or asynchronously, 
given a Page ID.
.RR "IO_WritePage" "IOreadwrite"
Write a page onto a physical device, either synchronously or asynchronously, 
given a Page ID.
.RR "IO_checker" "IOcheck"
Check that the volume header blocks on a volume are consistent.
.in 0

