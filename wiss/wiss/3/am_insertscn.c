
/********************************************************/
/*                                                      */
/*               WiSS Storage System                    */
/*          Version SystemV-4.0, September 1990	        */
/*                                                      */
/*              COPYRIGHT (C) 1990                      */
/*                David J. DeWitt 		        */
/*               Madison, WI U.S.A.                     */
/*                                                      */
/*	         ALL RIGHTS RESERVED                    */
/*                                                      */
/********************************************************/


#
/* Module : am_insertscan
      Inserts a record from 'recaddr' with length 'len' into the
      file associated with 'scanid', NEAR the current record of the scan.
  
   IMPORTS:
      SCANINFO *AM_getscan(scanid);
      int	st_appendrecord(OpenFileNum, RecAdr, Len, *NewRID, 
			trans_id, lockup, cond)
   EXPORTS:
      int	am_insertscan(scanid, recaddr, len, newrid)
*/

#include    <wiss.h>
#include    <am.h>

extern    SCANINFO *AM_getscan();

int
am_insertscan(scanid, recaddr, len, newrid)

int    scanid;	    /* id of this scan */
char   *recaddr;    /* buffer address of the new record */
int    len;	    /* length of the new record */
RID    *newrid;	    /* where to return the new rid */

/* Inserts a record from 'recaddr' with length 'len' into the
   file associated with 'scanid', NEAR the current record of the scan.
  
   RETURNS:
      the RID of the newly created record for SEQUENTIAL SCAN
  
   SIDE EFFECTS:
      NONE
  
   ERRORS:
      e3BADSCANTYPE - if scan type != SEQUENTIAL or INDEXSCAN
      e3ACCESSVIOLATION - the scan is opened for read-only
  
   BUGS:
      For a sequential scan, the record to be inserted is put NEAR the 
      current RID of the scan, but not necessarily immediately AFTER it.
*/

{
    register SCANINFO	*sptr;	/* address of scan info */
    RID	    	datarid;	/* used for st_appendrecord */
    char        buf[PAGESIZE];	/* record buffer */
    UPDRECORD	*uptr = (UPDRECORD *) buf;
    PID         page_of_rid;
    int e;	    /* error return value */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    	printf("am_insertscan(scanid=%d,recaddr=0x%x,len=%d,rid=0x%x)\n",
    	    	scanid, recaddr, len, newrid);
#endif

    sptr = AM_getscan(scanid);	/* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

    if (sptr->accessflag == READ)
    	return(e3ACCESSVIOLATION);	/* READ-only scan */

    switch(sptr->scantype)
    {
    	case INDEXSCAN:
    	case HASHSCAN:
    	    /* create a new record and save its RID */
    	    e = st_insertrecord(sptr->filenum, recaddr, len, &(sptr->rid), 
		  &datarid, sptr->trans_id, sptr->lockup, sptr->cond);
    	    CHECKERROR(e);
    	    uptr->type = INSERT;
    	    uptr->datarid = datarid;
    	    if (newrid != NULL) *newrid = datarid;

    	    movebytes(uptr->image, recaddr+(sptr->keyattr)->offset,
    	    	    (sptr->keyattr)->length);

    	    /* append it to the temporary file */
    	    e = st_appendrecord(sptr->deltafile, buf, 
		(sptr->keyattr)->length+UPDHEADERLEN, &datarid, sptr->trans_id,
		FALSE, sptr->cond);
	    CHECKERROR(e);
    	    break;

    	case SEQUENTIAL:
    	    e = st_insertrecord(sptr->filenum, recaddr, len, 
    	         &(sptr->rid), &datarid, sptr->trans_id, sptr->lockup, 
		 sptr->cond);
	    CHECKERROR(e);
    	    if (newrid != NULL) *newrid = datarid;
    	    break;

    	default:
    	    return (e3BADSCANTYPE);

    } /* switch */

    return(e);

}   /* am_insertscan */
