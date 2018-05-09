
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
/* Module : AM_commit1
        This module commits the updates (including inserts and deletes)
      of a hash update scan.  It is called when an update scan is closed.
  
   IMPORTS:
      st_readrecord(ofn, *RID, RecAdr, Len, trans_id, lockup, mode)
      st_writerecord(ofn, *RID, RecAdr, Len, trans_id, lockup)
      st_insertrecord(ofn, RecAdr, Len, *NearRID, *NewRID, trans_id, lockup)
      st_deleterecord(ofn, *RID)
      st_inserthash(hashofn, *Key, *dataRID, trans_id, lockup, cond)
      st_deletehash(hashofn, *Key, *dataRID, trans_id, lockup, cond)
      st_firstfile(dataofn, *RID, trans_id, lockup)
      st_nextfile(datafile, *CurrentRID, *NextRID, trans_id, lockup, mode)
      st_compare(ofn, *rid, operator, *keyattr, *value, trans_id, lockup)
  
   EXPORTS:
      int	AM_commit1(trans_id);
*/

#include    <wiss.h>
#include    <am.h>
#include     <lockquiz.h>

extern    SCANINFO *AM_getscan();

int
AM_commit1(scanid)
int scanid;


/* This routine commits the updates of a hash update scan.
   That is, records in the data file and possibly indices in the hash
   file are updated according to update (log) records in the "differential" 
   file.  During a hash update scan (scan opened with WRITE permission), 
   updates to records (and hence its index) are deferred util the the scan 
   is closed.
  
   RETURNS:
      None
  
   SIDE EFFECTS:
      None
  
   ERRORS:
      None
  
   BUGS:
      This routine does not automatically maintain other affected indices 
      (ie, fields on which other indices are built change values)
      on the same data file.
*/
{
    register SCANINFO   *sptr;
    RID	deltarid;	/* RID of an update record */
    RID	datarid;	/* RID of a data record */
    int	length;	    /* length of a record */
    int	e;	    /* for returned errors */
    KEY	newkey,key;	    /* key of the data record */
    char	buf[PAGESIZE];	/* buffer for records to be updated */
    UPDRECORD	*uptr = (UPDRECORD *)buf;
    int    datafile;	/* open file number of the data file */
    int    hashfile;	/* open file number of the hash file */
    int    deltafile;	/* open file number of the update (log) file */
    int	   trans_id;
    short  lockup;
    short  cond;

    sptr = AM_getscan(scanid);  /* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

#ifdef TRACE
    if (checkset (&Trace3, tCOMMIT))  
    {
    	printf("AM_commit1(dataofn=%d, hashofn=%d, updateofn=%d,\n",
    	    	datafile, hashfile, deltafile);
    	printf(" keyattr="); PRINTATTR(sptr->keyattr); printf("\n");
    }
#endif

    datafile = sptr->filenum;
    hashfile = sptr->indexfile;
    deltafile = sptr->deltafile;
    trans_id = sptr->trans_id;
    lockup = sptr->lockup;
    cond = sptr->cond;

    /* fill in key attributes for later use */
    key.length = sptr->keyattr->length;
    key.type = sptr->keyattr->type;
    newkey.length = key.length;
    newkey.type = key.type;

    /* loop through the whole update (log) file */

/* DeWitt:  I don't understand why the actual calls to st_firstfile and
 st_nextfile, and st_readrecord have lockup equal to FALSE */

    /* SHERROR st_firstfile(filenum, firstrid, trans_id, lockup) */
    /* SHERROR st_nextfile(filenum, ridptr, nextrid, trans_id, lockup, mode) */

    for (e = st_firstfile(deltafile, &deltarid,trans_id, FALSE, l_NL, cond); 
      e >= eNOERROR;
      e = st_nextfile(deltafile, &deltarid, &deltarid, trans_id, 
	FALSE, l_NL, cond))
    {
    	/* read in the next update record */

    /* SHERROR st_readrecord(filenum, ridptr, recaddr, reclen, trans_id, lockup, mode) */

    	length = st_readrecord(deltafile, &deltarid, buf, PAGESIZE, 
		trans_id, FALSE, l_NL, cond);
    	CHECKERROR(length);
    	length -= UPDHEADERLEN;	    /* actual record length */

    	switch(uptr->type)
    	{
    	    case INSERT:
    	    /* add a (key, RID) pair to the index file */
    	    movebytes(key.value, uptr->image, key.length);	
    	    e = st_inserthash(hashfile, &key, &(uptr->datarid), 
	    	sptr->trans_id, sptr->lockup, sptr->cond);
    	    CHECKERROR(e);
    	    break;	/* end of case INSERT */

    	    case DELETE:
    	    /* get the old key first */
    	    movebytes(key.value, uptr->image, key.length);	
    	    /* delete old (key,rid) pair */
    	    e = st_deletehash(hashfile, &key, &(uptr->datarid), 
	    	sptr->trans_id, sptr->lockup, sptr->cond);
    	    CHECKERROR(e);
    	    break;	/* end of case DELETE */

    	    case UPDATE:
    	    /* get the old key first */
    	    movebytes(key.value, uptr->image, key.length);	

    	    /* get the new key next */
    	    movebytes(newkey.value, uptr->image + key.length, 
    	    	      key.length);	

    	    movebytes(newkey.value, uptr->image, key.length);	

    	    /* delete old (key,rid) pair */
    	    e = st_deletehash(hashfile, &key, &(uptr->datarid), 
	    	sptr->trans_id, sptr->lockup, sptr->cond);
    	    CHECKERROR(e);

    	    /* add new (key, RID) pair to the index file */
    	    e = st_inserthash(hashfile, &newkey,&(uptr->datarid), 
	    	sptr->trans_id, sptr->lockup, sptr->cond);
    	    CHECKERROR(e);
    	    break;	/* end of case UPDATE */

    	}  /* end switch */

    }  /* end of for loop */

    return(eNOERROR);
}

