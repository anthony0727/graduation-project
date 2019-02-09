.ds f. storage.i
.ig
modified by Chou  (4/1/84)
..
.sh 2 "Indices"
.(x
.sh 2 "Indices"
.)x
.pp
WiSS supports two types of indices, B+-tree indices and hash indices.
In a B+-tree index, all the data (RIDs) are stored in the leaves, 
neighboring leaf pages are doubly linked to facilitate sequential 
scans on indices.
The structure of a hash index is similar to that of a B+-tree index.
Both structures are trees and use the same type of leaf pages.
The differences are that the root of a hash index is a hash table
consisting of SHORTPIDs, and a hash index uses overflow mechanism 
for storage re-organization.

.sh 3 "B-tree Structure"
.(x
.sh 3 "B-tree Structure"
.)x
.pp
The internal structure of a B-tree contains three types of pages,
the root, the node, and the leaf page (see Appendix \n+(AP).
These pages are similar in format except for a few differences.
All types of pages begin with a fixed amount of data space where page
entries are stored.  
This data space is followed by control information which is found
at the end of each page.  The control information contains the 
followings:
.np
File ID:
the internal file ID of the index file;
.np 
Page identifiers: 
this page, the next page, the previous page, and PID0;
.np 
Count information:
number of free bytes, number of entries;
.np 
Offset information:
start of data section (this will be zero for 
all node pages except for the root), end of the data section;
.np 
Miscellaneous:
type of key (int, char, etc.), type of page (root, node, leaf), 
type of B tree (index, link, or version),
uniqueness of index (primary or not);
.pp
A root page contains key attribute information about the tree.
This information is referred to as
.ul
header information,
which is a triplet <key type, key length, offset into record>.
The internal node page format of the B-tree is the same as that of
the root page except there is no header information.
The rest of a root or a internal page are filled with key/pointer pairs.
The format of key/pointer pairs is:
.sp
.ce
< key (length, value),  pointer (PID) >
.pp
The keys may be variable or fixed length.
If the key is numeric (int, float, etc) then it will be of fixed length.
If the key is of type character, then we will probably have to deal with
variable length keys.
It is possible that a key may be of type char and be a string of 50 characters.
We don't want to have to store this entire key in the internal node page 
because it would drastically cut down on the fanout of the tree.
Since the entire key is needed only in the leaf nodes and the internal nodes
use the keys only for searching, we can use a prefix of the actual key.
This technique is known as
.ul
prefix B trees.
We will call these unique prefixes 
.ul
separators.
The prefix property can be stated thusly:
.in 4
Let x and y be any two keys such that x < y. 
Then there is a unique prefix y' of y such that
.ti 1
a) y' is a separator between x and y.
.ti 1
b) no other separator between x and y is shorter than y'.
.pp
The leave page format is similar to that of the internal node pages,
except, instead of key/pointer pairs, there are key/RID_list groups.
The format of a key/RID_list entry is:
.sp
.ce
< key (length, value), RID count, RID list ... >
.sp
For secondary indices, there may be more than one records with the same key.
This implies that there may be potentially any number of RIDs associated
with a single key.
Therefore we must have some way to deal with very long RID lists.
Instead of adopting the overflow page technique,
long data items are used to store long RID lists.
The RID of the long item directory and the negated RID count
are placed where an ordinary key/RID_list pair would go.

.sh 3 "B+-tree Routines"
.(x
.sh 3 "Routines"
.)x
.de RR
.np
\\$1  (in \\$2.c)
.sp 0
..
.RR "bt_binary_search" "btutil1"
Search a key on a B-tree page, and return its relative
position (slot number) on the page. This routine returns an
error code if the key does not exist on the page, and the
slot number returned is the position where the key would be.
.RR "bt_allocpage" "btutil1"
Allocates a new B-tree page with control information initialized.
.RR "bt_insertentry" "btutil1"
Inserts a new entry into a B-tree page.
.RR "bt_deletentry" "btutil1"
Deletes an entry from a B-tree page.
.RR "bt_compress_page" "btutil1"
Compress a B-tree page by removing the holes on it.
.RR "bt_move_entries" "btutil2"
Move entries from one B-tree page to another.
.RR "bt_split_page" "btutil2"
Split a B-tree page into two and reallocate entries on
the old page. The new page is always added to the right of
the old page.
.RR "bt_balance_pages" "btutil2"
Redistribute entires on two B-tree pages so that they will be
more evenly spread.
.RR "bt_merge_pages" "btutil2"
Merge two B-tree pages into one, and relase the empty one.
.RR "bt_leftmost" "btutilx"
Given the root of a subtree, locate its leftmost leaf page.
.RR "bt_rightmost" "btutilx"
Given the root of a subtree, return its rightmost leaf page.
.RR "bt_prefixkey" "btutilx"
Given a key and a B-tree node page on which to insert it,
determined the length of the shortest prefix that is sufficient
to distinguish that key from the largest key in the  subtree
of the page's left neighbor. This is done by traversing down the
subtree of the left neighbor and compare the key
with the largest key on the rightmost leaf of the neighbor.
.RR "bt_getrid" "btleafrids"
Return the RID identified by a structure of the form
<leaf_page, slot_num, offset>.
"Offset" is the offset of the RID into the RID list of 
entry <leaf_page, slot_num>.
.RR "bt_addrid" "btleafrids"
Append a RID to the RID list of entry <leaf_page, slot_num>.
.RR "bt_zaprid" "btleafrids"
Locate the position of a RID on a RID list, then remove it.
.RR "bt_insertkey" "btinsertkey"
Insert (recursively) a key/pointer pair into a parent page.
.RR "bt_traverse" "bttraverse"
Locate the position of a key in a B-tree.
If the key is found, the corresponding slot number on a leaf page is returned;
otherwise, an error code and the slot where the key should be in are returned.
.RR "bt_print_btfile" "btdump"
Given the root of a B-tree (or subtree), print the B-tree.
.in 0

.sh 3 "Hash Index Structure"
.(x
.sh 3 "Hash Index Structure"
.)x
.pp
The structure of a hash index is similar to that of a B+-tree index.
They use the same type of leaf pages, and even the same routines 
to locate and manipulate entries on a leaf page.
However, the root of a hash index is a hash table
consisting of SHORTPIDs, each of which points to a primary bucket (leaf page).
When the root of a hash index is less than a page,
extendible hashing mechanism is used to manage storage re-organization;
beyond that, overflow chain mechanism is employed.

.sh 3 "Hash Index Routines"
.(x
.sh 3 "Hash Index Routines"
.)x
.RR "h_dumpfile" "h_dump"
Print in a readable form the contents of a hash index.
This is for debugging purpose only.
.RR "h_hash" "h_util"
This is a dynamic hashing function that maps a key into a bit string,
whose length is an input parameter to the routine.
.RR "h_initpage" "h_util"
This initializes a leaf page for a hash index.

