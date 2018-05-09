.ig
last edited by
chou (6/15/85)
..
.nr AP 0 1 \"appendix numbering
.nr FG 0 1 \"figure numbering

.bp
.po 1.25i
.(b
.sp 10
.)b
.ps 12
.ce 100
Implementation Notes for the Wisconsin Storage System

Version 4.0

.ps 10
WiSS Implementation Team
Computer Sciences Department
University of Wisconsin
Madison, WI 53706


.pn 1
.bp

.po 1.25i
.fo 'WiSS Implementation Notes'-%-'1985 June'

.ce 100
Implementation Notes for the Wisconsin Storage System

WiSS Implementation Team
Computer Sciences Department
University of Wisconsin
Madison, WI 53706

.sh 1 "Introduction"
.(x
.sh 1 "Introduction"
.)x
.pp
This paper provides documentation on the internal details of
of the Wisconsin Storage System.
This includes design decisions, and descriptions of 
the data structures and the routines in each level 
of WiSS which are exported between individual modules in that level.
These routines are used in various combinations by the routines which
provide an interface to each level; for descriptions of those routines,
see [WDes].

.so io.nr
.so buffer.nr
.so storage.nr
.so access.nr
.so trace.nr

.fo  ''''
.ls 1
.bp
.sh 0 "Table of Contents"
.xp

.bp
.sh 0 "References"

.in 4
.ti 0
[Astr] Astrahan et al, "System R: Relational Approach to Database Management".
ACM Transactions on Data Systems 1:2 (1976 June) pp. 119-120.

.ti 0
[Cham] Chamberlin et al,
"A History and Evaluation of System R".
Communications of the ACM 24:10 (1981 October) pp. 639.

.ti 0
[Fagi] Fagin et al,
"Extendible Hashing-A Fast Access Method for Dynamic Files".
ACM Transactions on Data Systems 4:3 (1979 September) pp. 315-344

.ti 0
[Hask] Haskin, et al,
"On Extending the Functions of a Relational Database System".
IBM Technical Report, 1981? [IBM Research Laboratory, San Jose CA].

.ti 0
[Marc] March, Severance, Wilens,
"Frame Memory: A Storage Architecture to Support Rapid Design and
Implementation of Efficient Databases".
ACM Transactions on Data Systems 6:3 (1981 September) pp. 441-463.

.ti 0
[WBtr] WiSS Implementation Team,
"B-Tree Structures in the Wisconsin Storage System".
University of Wisconsin, 1982.

.ti 0
[WDes] WiSS Implementation Team,
"Design Specifications of the Wisconsin Storage System".
University of Wisconsin, 1984.
.in 0

.so append.nr

