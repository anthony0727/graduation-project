.nr AP 0 1

.bp
.sp 3
.ce 100
Appendix \n+(AP. Volume Header Structure
.sp 4
.hl
.cs R 24
.ls 1
.tr -\(em
.tr @
.nf


+----------------------------+
|       Volume Title         |
|----------------------------|
|          VolID             |
|----------------------------|
|      Pages per Extent      |
|----------------------------|
|      Number of Extents     |
|----------------------------|
|   Number of Free Extents   |
|----------------------------|
|  Maximum Number of Files   |
|----------------------------|
|  Number of Existing Files  |
|----------------------------|
|   Root Page for FileDir    |
|============================|
|      Free Extent Map       |
|============================|
|    Bad Page Remap List     |
|----------------------------|
|      Bad Page Count        |
+----------------------------+


.cs R
.tr --

.hl

.bp
.sp 3
.ce
Appendix \n+(AP. Data Page Format


.hl

.cs R 24
.tr -\(em
.nf

|-----------------------------------------------------------|
|                Data                                       |
|                             /-----------------------------|
|----------------------------/                              |
|                                  Free                     |
|-----------------------------------------------------------|
|  Ptr RIDCnt-1   | . . .  | PtrRID1  | PtrRID0  | ThisPage |
|-----------------|--------|----------|----------|----------|
| First Free Byte | FileID |  RIDCnt  | PrevPage | NextPage |
|-----------------------------------------------------------|

.sp 2
.cs R

.hl

.bp
.sp 3
.ce
Appendix \n+(AP. B+tree Page Formats



.hl



.ce
\fBI.\fI Root and Node Page Format \fR

.cs R 24
.tr -\(em
.nf
.ce 15
+--------------------------------+
|[header information]|PID 1:key 1|
+--------------------------------+
|PID 2:key 2|   ...  |PID i:key i|
+--------------------------------+
|         Free space             |
+--------------------------------+
| offset i |    ...    | offset2 |
+--------------------------------+
| offset 1 | control information |
+--------------------------------+
.sp 2
.cs R

.ce
\fBII. \fILeaf Page Format \fR
.cs R 24
.tr -\(em
.tr @
.nf

.ce 15
+----------------------------------+
|data entry 1|data entry 2| . . .  |
|----------------------------------|
|    . . . | data entry i |        |
|-------------------------+        |
|         free space               |
|----------------------------------|
|offset i| . . . |offset 2|offset 1|
|----------------------------------|
|      control information         |
+----------------------------------+
.cs R
.tr --



.hl


.bp
.sp 3

.ce
Appendix \n+(AP. File Directory Structure




.hl

.cs R 24
.tr -\(em
.nf

  +---+        +---+
  | d |        |d-1|
 +----+        +---+--------+
 |PTR0|------->|            |
 +----+        |  BUCKET  +-+
 |PTR1|------->|          |.|-+
 +----+        +----------+-+ |
 | :  |                       =
 +----+
 | :  |        +---+
 +----+        | d |
 | :  |        +---+------------+  +-----------------+
 +----+        |                |  |                 |
 |PTRi|------->| PRIMARY PAGE +-+  | OVERFLOW PAGE +-+
 +----+        |              |.|->|               |.|--+
 | :  |        +--------------+-+  +---------------+-+  |
 +----+                                                 =
.sp 2
.cs R
.tr --


.hl

.bp
.sp 3
.ce
Appendix \n+(AP. Directory Page Formats


.hl

.ce
\fBI. \fI Layout of Root Page (Hash Table) \fR
.cs R 24
.tr -\(em
.nf

.ce 10
+-----------------------------------+
| PID0  | PID1  | PID2  | ... | ... |
+-------+-------+-------+-----+-----+
|     ROOT PAGE (HASH TABLE)        |
|(# of entries = 2 ** global depth) |
+-------+-------+-------+-----+-----+
| NULL  | NULL  | NULL  | ... | ... |
+-----------------------------------+
.sp 2
.cs R



\fBII. \fIBucket Page Format \fR
.cs R 24
.tr -\(em
.nf

.ce 15
+-------------------------------------------------+
|File Descriptor 1|File Descriptor 2| ...         |
+-----------------+-----------------+-------------+
|File Descriptor n|                               |
+-----------------+                               |
|                          Free  Area             |
+-----------+------------+------+--------+--------+
|Local Depth|FileCount(n)|FileID|ThisPage|NextPage|
+-------------------------------------------------+

.cs R
.tr --


.hl


