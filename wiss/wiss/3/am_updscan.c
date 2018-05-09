
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
/* Module : am_updatescan
      updates the current record of a scan

   IMPORTS:
      int	st_writerecord(OpenFileNum, *RID, RecAdr, Len, 
    			trans_id, lockup, cond)
      int	st_appendrecord(OpenFileNum, RecAdr, Len, *NewRID, 
			trans_id, lockup, cond)
      int	st_readfield (OpenFileNum, *RID, Fieldaddr, Fieldoffset,
			FieldLen, trans_id, lockup, mode, cond)
      SCANINFO *AM_getscan(scanid)

   EXPORTS:
      int	am_updatescan(ScanID, RecAdr, Len)
*/
  
#include    <wiss.h>
#include    <am.h>
#include     <lockquiz.h>

extern    SCANINFO *AM_getscan();

int
am_updatescan(scanid, recadr, len)
int    scanid;	    /* identifies a particular scan */
char    *recadr;	/* buffer address of the new record image */
int    len;	    /* length of the new image */

/* This routine updates the current record of the scan
       associated with scanid.
  
    Returns:
      None
  
    Side effects generated here:
      None
  
    Errors generated here:
      e3BADSCANTYPE - if scan type = LONG
      e3ACCESSVIOLATION - the scan is opened for read-only
*/
  
{
    register SCANINFO	*sptr;	/* address of scan info record */
    register int	e;	/* for returned errors */
    RID	    dummy;	/* used for st_appendrecord */
    char	    buf[PAGESIZE];	/* buffer for update record */
    UPDRECORD	*uptr = (UPDRECORD *) buf;
    int	    keyoffset, keylength;

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    	printf("am_updatescan(scanid = %d)\n", scanid);
#endif

    sptr = AM_getscan(scanid);	/* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);
    if (sptr->accessflag == READ)
    	return(e3ACCESSVIOLATION);	/* read-only scan */

    switch(sptr->scantype)
    {
    	case INDEXSCAN:
    	case HASHSCAN:
    	    /* examine the key field of the record to see
    	    if it has changed.  If so add the new and
    	    old key values to the deltafile and then
    	    write  the modified record back into the database */
      	    keyoffset = (sptr->keyattr)->offset;
    	    e= st_compare(sptr->filenum, &sptr->rid, EQ, sptr->keyattr, 
	        recadr+keyoffset, sptr->trans_id, sptr->lockup, sptr->cond);
    	    CHECKERROR(e);
    	    if (e != TRUE)
    	    {
    	        /* key has changed, get the old key, and save
    	         it on the uptr->image,  and then save the new key 
    	         on uptr->image immediately afterward */

    	        keylength = (sptr->keyattr)->length;

    	        /* save old key on uptr->image */
    	        st_readfield(sptr->filenum, &sptr->rid, uptr->image, 
    	             keyoffset, keylength, sptr->trans_id, sptr->lockup, l_S, 
	             sptr->cond);

    	        /* save the new key */
    	        movebytes(uptr->image + keylength, recadr+keyoffset, keylength);

    	        /* construct the update record (of type UPDATE) */
    	        uptr->type = UPDATE;
    	        uptr->datarid = sptr->rid;

    	        /* insert it into the temporary file */
    	        e = st_appendrecord(sptr->deltafile, buf, 
		    (2*keylength) + UPDHEADERLEN, &dummy, sptr->trans_id, FALSE,
		    sptr->cond);
    	    }

    	    /* write the modified record to the file */
    	    e = st_writerecord(sptr->filenum, &sptr->rid, recadr, len, 
    	    	    sptr->trans_id, sptr->lockup, sptr->cond); 
    	    CHECKERROR(e);
    	    break;

    	case SEQUENTIAL:
	    /* update the record */
    	    e = st_writerecord(sptr->filenum, &sptr->rid, recadr, len, 
    	    	    sptr->trans_id, sptr->lockup, sptr->cond);	
    	        CHECKERROR(e);
    	    break;

    	default:
    	    return (e3BADSCANTYPE);
    }

    return(e);	/* return any possible error code */

} /* am_updatescan */
