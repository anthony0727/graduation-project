
/* errors from level 2,  file directory and open file table, sections */

case e2VOLNOTOPEN:
	return("volume not mounted at level 2");
	break;

case e2NOSUCHFILE:
	return(" no file by that name ");
	break;

case e2VOLALREADYOPEN:
	return("volume already open at level 2 ");
	break;

case e2FILEALREADYEXISTS:
	return(" this file already exist");
	break;

case e2NOMOREMEMORY:
	return(" calloc failed ");
	break;

case e2TOOMANYVOLUMES:
	return(" too many volumes currently in use ");
	break;

case e2FILESTILLOPEN:
	return(" attempt to remove a currently open file ");
	break;


case e2BADOPENFILENUM:
	return(" openfilenum is out of range ");
	break;

case e2WRONGUSER:
	return(" this openfilenum belongs to another user ");
	break;

case e2TOOMANYOPENFILES:
	return(" too many open files ");
	break;

case e2UNKNOWNMODE:
	return("unknown access mode");
	break;

case e2MODECONFLICT:
	return("access modes conflict");
	break;

case e2UNKNOWNPROT:
	return("unknown protection mode");
	break;

case e2PERMISSIONDENIED:
	return("file permission denied");
	break;

case e2NOPERMISSION:
	return("no file permission");
	break;


