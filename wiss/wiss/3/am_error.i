

/* errors from level 3, */

case e3BADCURSOR:
	return("'bad scan cursor'");
	break;

case e3BADSCANTYPE:
	return("'scan type incompatible with operation'");
	break;

case e3BADRELOCATION:
	return("'incorrect relocation'");
	break;


case e3NOFILENO:
	return("'file number not found'");
	break;

case e3BADFILENO:
	return("'invalid file number'");
	break;

case e3BADSCANID:
	return("'invalid scanid'");
	break;

case e3SCANFILENOTMATCH:
	return("'the scanid does not belong with the file number'");
	break;

case e3NOMOREMEMORY:
	return("'no more memory space for scan table'");
	break;

case e3ACCESSVIOLATION:
	return("'attempt to update a read-only file'");
	break;

case e3NONEXTRID:
	return("'no more next RID - end of file'");
	break;

case e3NOPREVRID:
	return("'no more previous RID - end of file'");
	break;

