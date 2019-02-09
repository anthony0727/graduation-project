
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
/* Module : am_deletescan - deletes the current record of a scan
  
   IMPORTS:
      int	st_deleterecord(OpenFileNum, *RID)
      int	st_appendrecord(OpenFileNum, RecAdr, Len, *NewRID, trans_id, lockup)
      SCANINFO *AM_getscan(scanid)
  
   EXPORTS:
      int	am_deletescan(scanid)
*/

#include    <wiss.h>
#include    <am.h>
#include     <lockquiz.h>

extern    SCANINFO *AM_getscan();

int
am_deletescan(scanid)
int      scanid;   /* identifies a particular scan */

/* This routine deletes the current record of the scan associated with scanid.
   If this is an index scan, deletion of the index entry is deferred and 
   recorded in a file.
      
    Returns:
      None
  
    Side effects generated here:
      None
  
    Errors generated here:
      e3BADSCANTYPE - if scan type = LONG
      e3ACCESSVIOLATION - if the scan is for read-only
*/

{
    register SCANINFO	*sptr;	/* address of scan info record */
    register int	e;	/* for returned errors */
    RID	    dummy;	/* used for st_appendrecord */
    PID             mypid;
    char	    buf[UPDHEADERLEN];
    UPDRECORD	*uptr = (UPDRECORD *) buf;
    int	    keyoffset, keylength;

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    	printf("am_deletescan(scanid = %d)\n", scanid);
#endif

    sptr = AM_getscan(scanid);	/* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

    if (sptr->accessflag == READ) return(e3ACCESSVIOLATION);

    switch(sptr->scantype)
    {
    	case LONGSCAN:
    	    return (e3BADSCANTYPE);

    	case SEQUENTIAL:
    	    e = st_deleterecord(sptr->filenum, &sptr->rid, 
    	    	sptr->trans_id, sptr->lockup, sptr->cond);
/* DeWitt:  Shahram had previous statement comment out and was using
the next statement. Why? */
    	    /* e = st_scan_delrec(sptr, sptr->trans_id, sptr->lockup); */
    	    CHECKERROR(e);
    	    break;

    	case INDEXSCAN:
    	case HASHSCAN:
    	    /* construct the update (DELETE) record */
    	    uptr->type = DELETE;
    	    uptr->datarid = sptr->rid;
    	    keylength = (sptr->keyattr)->length;
      	    	keyoffset = (sptr->keyattr)->offset;

    	    /* save old key on uptr->image */
    	    if (sptr->lockup)
    	    {
    	        GETPID(mypid, sptr->rid);
    	        if PIDEQ(sptr->locked_page,mypid)
    	            st_readfield(sptr->filenum, &sptr->rid, uptr->image, 
    	            keyoffset, keylength, sptr->trans_id, FALSE, l_X, 
		    sptr->cond);
    	        else
    	        {
    	    	    GETPID(sptr->locked_page,sptr->rid);
    	            st_readfield(sptr->filenum, &sptr->rid, uptr->image, 
    	            keyoffset, keylength, sptr->trans_id, sptr->lockup, 
		    l_X, sptr->cond);
    	        }

    	        /* append it to the temporary file */
    	        e = st_appendrecord(sptr->deltafile, buf, 
		      UPDHEADERLEN+keylength, &dummy, sptr->trans_id, 
		      FALSE, sptr->cond);

    	        e = st_deleterecord(sptr->filenum, &sptr->rid, 
			sptr->trans_id, FALSE, sptr->cond);
    	    }
    	    else
    	    {
    	        st_readfield(sptr->filenum, &sptr->rid, uptr->image, 
    	            keyoffset, keylength, sptr->trans_id, sptr->lockup, 
			l_X, sptr->cond);

    	        /* append it to the temporary file */
    	        e = st_appendrecord(sptr->deltafile, buf, 
    	               UPDHEADERLEN+keylength, &dummy, sptr->trans_id, 
		       FALSE, sptr->cond);

    	        e = st_deleterecord(sptr->filenum, &sptr->rid, sptr->trans_id, 
			FALSE, sptr->cond);
    	    }
    	    CHECKERROR(e);
    	    break;

    }  /* end of switch */

    return(e);

} /* am_deletescan */
