
/* errors from level 2, records sections */

case e2NAMEINUSE:
	return("file name already in use");
	break;

case e2BADSLOTNUMBER:
	return("an invalid slot number");
	break;

case e2LISTOFEMPTYSLOTS:
	return("empty slots at end of page");
	break;
	/* an inconsistency */
case e2NOROOMONPAGE:
	return("no room on page for expansion");
	break;


case e2NULLRIDPTR:
	return("null RID pointer");
	break;

case e2NULLPIDPTR:
	return("null PID pointer");
	break;

case e2NULLFIDPTR:
	return("null FID pointer");
	break;

case e2NULLPAGEPTR:
	return("null page buffer pointer ");
	break;

case e2NULLRECADDR:
	return("null record buffer address");
	break;

case e2DELNOEXTREC:
	return("delete a non-existing record");
	break;

case e2PAGENOTINFILE:
	return("the page referenced is not in the file");
	break;


case e2ENDOFFILE:
	return("end of file in st_nextfile/st_prevfile");
	break;

case e2RECWONTFIT:
	return("no room for record on this page");
	break;

case e2CANTINSERTREC:
	return("st_insertrecord fails for unknown reason");
	break;

case e2VOLUMESTILLACTIVE:
	return("attempt to dismount a volume with open files");
	break;


case e2NOMORESLICES:
	return("long data item grows exceed limit");
	break;

case e2BADDATATYPE:
	return("bad data type in st_compare");
	break;

case e2ILLEGALOP:
	return("illegal operator found in st_compare");
	break;

