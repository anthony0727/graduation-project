
/* errors from level 2, b tree section */

E(e2NONEXTKEY, -122,	`No next index')
E(e2NOPREVKEY, -222,	`No previous index')
E(e2NOFIRSTINDEX, -322,	`No first index')
E(e2NOLASTINDEX, -422,	`No last index')
E(e2NONEXTRID, -522,	`CurrRID is the last RID for this key')
E(e2NOPREVRID, -622,	`CurrRID is the first RID for this key')

E(e2DUPLICATEKEY, -123,	`Duplicate key found during construction of a primary index')
E(e2DUPLICATEKEYPTR, -223,`Duplicate key found in node pages of the B-tree')

E(e2KEYNOTFOUND, -323,	`cannot find the key')

E(e2INDEXNUMTOOLARGE, -128,	`Index number too large (createindex)')
E(e2KEYLENGTHTOOLONG, -228,	`Key too long (createindex)')
E(e2BADSLOTCOUNT, -328,		`negative slot count in bt_move_entries')

E(e2KEYALREADYEXISTS, -428,	`Key already exists.')
E(e2FILENAMETOOLONG, -528,	`File name too long. DUPLICATE ERROR CODE.')
E(e2NORIDMATCH, -628,		`the index to be deleted is not found')
E(e2ILLEGALCURSOR, -728,	`bad cursor for index scan')

