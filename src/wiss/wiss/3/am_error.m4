
/* errors from level 3, */

E(e3BADCURSOR, -132, 'bad scan cursor')
E(e3BADSCANTYPE, -232, 'scan type incompatible with operation')
E(e3BADRELOCATION, -332, 'incorrect relocation')

E(e3NOFILENO,-134,'file number not found')
E(e3BADFILENO,-234,'invalid file number')
E(e3BADSCANID,-334,'invalid scanid')
E(e3SCANFILENOTMATCH,-434,'the scanid does not belong with the file number')
E(e3NOMOREMEMORY,-534,'no more memory space for scan table')
E(e3ACCESSVIOLATION, -634,'attempt to update a read-only file')
E(e3NONEXTRID, -734, 'no more next RID - end of file')
E(e3NOPREVRID, -834, 'no more previous RID - end of file')
