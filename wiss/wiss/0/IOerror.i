

/* error codes returned by level 0 */

case e0DEVSEEKERROR:
	return("device seek error");
	break;

case e0DEVREADERROR:
	return("device read error");
	break;

case e0DEVWRITEERROR:
	return("device write error");
	break;

case e0MOUNTFAILED:
	return("device mount failed");
	break;

case e0DISMOUNTFAILED:
	return("device dismount failed");
	break;


case e0VOLNOTMOUNTED:
	return("referenced volume not mounted");
	break;

case e0TOOMANYVOLS:
	return("too many volumes mounted");
	break;

case e0TOOMANYFILES:
	return("too many files created");
	break;

case e0NOSPACEONDISK:
	return("insufficient space on disk");
	break;

case e0NOMOREMEMORY:
	return("insufficient main memory");
	break;

case e0VOLMOUNTED:
	return("referenced volume is mounted ");
	break;


case e0FIDPIDNOTMATCH:
	return("volume IDs in the FID and PID are inconsistent");
	break;

case e0FILENOTINUSE:
	return("reference to a non-existent file");
	break;

case e0INVALIDPID:
	return("invalid page number");
	break;

case e0INVALIDFID:
	return("invalid file number");
	break;

case e0BADHEADER:
	return("bad volume header");
	break;


case e0NULLPIDPTR:
	return("null PID pointer");
	break;

case e0NULLFIDPTR:
	return("null FID pointer");
	break;

case e0NULLBUFPTR:
	return("null memory buffer pointer");
	break;

