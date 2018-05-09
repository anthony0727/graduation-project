

/* errors from level 2, b tree section */

case e2NONEXTKEY:
	return("No next ");
	break;

case e2NOPREVKEY:
	return("No previous ");
	break;

case e2NOFIRSTINDEX:
	return("No first ");
	break;

case e2NOLASTINDEX:
	return("No last ");
	break;

case e2NONEXTRID:
	return("CurrRID is the last RID for this key");
	break;

case e2NOPREVRID:
	return("CurrRID is the first RID for this key");
	break;


case e2DUPLICATEKEY:
	return("Duplicate key found during construction of a primary ");
	break;

case e2DUPLICATEKEYPTR:
	return("Duplicate key found in node pages of the B-tree");
	break;


case e2KEYNOTFOUND:
	return("cannot find the key");
	break;


case e2INDEXNUMTOOLARGE:
	return("Index number too large (createindex)");
	break;

case e2KEYLENGTHTOOLONG:
	return("Key too long (createindex)");
	break;

case e2BADSLOTCOUNT:
	return("negative slot count in bt_move_entries");
	break;


case e2KEYALREADYEXISTS:
	return("Key already exists.");
	break;

case e2FILENAMETOOLONG:
	return("File name too long. DUPLICATE ERROR CODE.");
	break;

case e2NORIDMATCH:
	return("the  to be deleted is not found");
	break;

case e2ILLEGALCURSOR:
	return("bad cursor for  scan");
	break;


