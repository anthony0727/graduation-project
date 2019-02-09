
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
/* Module : AM_commit
        This module commits the updates (including inserts and deletes)
      of an index update scan.  It is called when an update scan is closed.
  
   IMPORTS:
      st_readrecord(ofn, *RID, RecAdr, Len, trans_id, lockup, mode, cond)
      st_writerecord(ofn, *RID, RecAdr, Len, trans_id, lockup, cond)
      st_insertrecord(ofn, RecAdr, Len, *NearRID, *NewRID, 
    	trans_id, lockup, cond)
      st_deleterecord(ofn, *RID, trans_id, lockup, cond)
      st_insertindex(indexofn, *Key, *dataRID)
      st_deleteindex(indexofn, *Key, *dataRID)
      st_firstfile(dataofn, *RID, trans_id, lockup, mode, cond)
      st_nextfile(datafile, *CurrentRID, *NextRID, trans_id, lockup, 
    	mode, cond)
      st_compare(ofn, *rid, operator, *keyattr, *value, trans_id, 
    	lockup, cond)
  
   EXPORTS:
      int	AM_commit(scanid)
*/

#include    <wiss.h>
#include    <am.h>
#include    <lockquiz.h>


extern    SCANINFO *AM_getscan();

int
AM_commit(scanid)
int  scanid;

/* This routine commits the updates of an index update scan.
   That is indices in the index file are updated according to 
   update (log) records in the "differential" 
   file.  During an index update scan (scan opened with WRITE permission), 
   updates to indices are deferred util the the scan is closed.
  
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

    (DEWITT, 1/86).  In addition, this code really does not work
    the way it should.  IF a file is clustered on an attribute,
    and the clustering attribute is modified, the records are 
    not kept clustered.
*/
{
    register SCANINFO   *sptr;
    RID		deltarid;	/* RID of an update record */
    RID		datarid;	/* RID of a data record */
    int		length;	    /* length of a record */
    int		e;	    /* for returned errors */
    KEY		key;	    /* key of the data record */
    KEY		newkey;	    /* key of the data record */
    char	buf[PAGESIZE];	/* buffer for records to be updated */
    UPDRECORD	*uptr = (UPDRECORD *)buf;
    int     	*key11;
    int		datafile;
    int		indexfile;
    int		deltafile;
    int		trans_id;
    short	cond;
    short	lockup;

    sptr = AM_getscan(scanid);  /* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

    datafile = sptr->filenum;
    indexfile = sptr->indexfile;
    deltafile = sptr->deltafile;
    trans_id = sptr->trans_id;
    lockup = sptr->lockup;
    cond = sptr->cond;

#ifdef TRACE
    if (checkset (&Trace3, tCOMMIT))  
    {
    	printf("AM_commit(dataofn=%d, indexofn=%d, updateofn=%d,\n",
    	    	datafile, indexfile, deltafile);
    	printf(" keyattr="); PRINTATTR(sptr->keyattr); printf("\n");
    }
#endif

    /*printf ("\t Enterin AM_commit.\n");*/
    /* fill in key attributes for later use */
    key.length = sptr->keyattr->length;
    key.type = sptr->keyattr->type;
    newkey.length = key.length;
    newkey.type = key.type;

    if (lockup)
    {
        lock_file(trans_id, F_FILEID(indexfile), l_X, COMMIT, cond);
    }

    /* loop through the whole update (log) file */
    for (e = st_firstfile(deltafile, &deltarid, trans_id, FALSE, l_NL, cond);
        e >= eNOERROR;
        e = st_nextfile(deltafile, &deltarid, &deltarid, trans_id, FALSE, 
		l_NL, cond))
    {
    	/* read in the next update record */
    	length = st_readrecord(deltafile, &deltarid, buf, PAGESIZE, 
    	    trans_id, FALSE, l_NL, cond);
    	CHECKERROR(length);
 
    	switch(uptr->type)
    	{
    	    case INSERT:
    	    /* add a (key, RID) pair to the index file */
    	    movebytes(key.value, uptr->image, key.length);	
    	    e = st_insertindex(indexfile, &key, &(uptr->datarid), 
    	    	 trans_id, l_NL, cond);
    	    CHECKERROR(e);
    	    break;	/* end of case INSERT */

    	    case DELETE:
    	    /* get the old key first */
    	    movebytes(key.value, uptr->image, key.length);	
    	    /* delete old (key,rid) pair */
    	    e = st_deleteindex(indexfile, &key, &(uptr->datarid), 
    	    	 trans_id, l_NL, cond);
    	    CHECKERROR(e);
    	    break;	/* end of case DELETE */

    	    case UPDATE:
    	    /* get the old key first */
    	    movebytes(key.value, uptr->image, key.length);	

    	    /* get the new key next */
    	    movebytes(newkey.value, uptr->image + key.length, 
    	    	      key.length);	

    	    /* movebytes(newkey.value, uptr->image, key.length); */

    	    /* delete old (key,rid) pair */
    	    e = st_deleteindex(indexfile, &key, &(uptr->datarid),
    	    	 trans_id, l_NL, cond);
    	    if (e == ABORTED) return(eNOERROR);
    	    CHECKERROR(e);

    	    /* add new (key, RID) pair to the index file */
    	    e = st_insertindex(indexfile, &newkey,&(uptr->datarid),
    	    	 trans_id, l_NL, cond);
    	    if (e == ABORTED) return(eNOERROR);
    	    CHECKERROR(e);
    	    break;	/* end of case UPDATE */

    	}  /* end switch */

    }  /* end of for loop */

    return(eNOERROR);

}    /* AM_commit */
