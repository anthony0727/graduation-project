
case e1PAGENOTFOUND:
	return("page not found in buffer pool");
	break;

case e1NOFREEBUFFERS:
	return("no more free buffers");
	break;

case e1WRONGBUFFER:
	return("accessing the wrong buffer");
	break;

case e1NULLFIDPARM:
	return("null FID parameter");
	break;

case e1NULLPIDPARM:
	return("null PID parameter");
	break;

case e1BADMODEPARM:
	return("bad file mode parameter");
	break;

case eNOMOREBUCKETS:
	return("no more hash buckets");
	break;

case eNOENTRY:
	return("no entry for deletion");
	break;

case e1PAGEWASFOUND:
	return("page was already in buffer pool");
	break;

case e1PAGEWASFIXED:
	return("page was still fixed in buffer pool");
	break;

