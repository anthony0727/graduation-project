/* errors from level 2,  file directory and open file table, sections */

E(e2VOLNOTOPEN,-4021,`volume not mounted at level 2')
E(e2NOSUCHFILE,-4121,` no file by that name ')
E(e2VOLALREADYOPEN,-4221,`volume already open at level 2 ')
E(e2FILEALREADYEXISTS,-4321,` this file already exist')
E(e2NOMOREMEMORY,-4421,` calloc failed ')
E(e2TOOMANYVOLUMES,-4521,` too many volumes currently in use ')
E(e2FILESTILLOPEN,-4621,` attempt to remove a currently open file ')

E(e2BADOPENFILENUM,-3021,` openfilenum is out of range ')
E(e2WRONGUSER,-3121,` this openfilenum belongs to another user ')
E(e2TOOMANYOPENFILES,-3221,` too many open files ')
E(e2UNKNOWNMODE,-3321,`unknown access mode')
E(e2MODECONFLICT,-3421,`access modes conflict')
E(e2UNKNOWNPROT,-3521,`unknown protection mode')
E(e2PERMISSIONDENIED,-3621,`file permission denied')
E(e2NOPERMISSION, -3721, `no file permission')

