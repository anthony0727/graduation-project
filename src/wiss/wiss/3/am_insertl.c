
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
/* Module : am_insertlong
      Inserts 'length' bytes from 'recaddr' to the long data item
      associated with 'scanid'.
  
   IMPORTS:
      int	st_insertframe(OpenFileNum, DirRID, Cursor, RecAddr, 
    			Length, trans_id, lockup, cond)
      SCANINFO *AM_getscan(scanid);
      int	AM_nextscan(openfileno, scanid)
      
   EXPORTS:
      int	am_insertlong(scanid, where, recaddr, length)
*/

#include    <wiss.h>
#include    <am.h>
  
extern    SCANINFO *AM_getscan();

int
am_insertlong(scanid, where, recaddr, length)
int     scanid;	      /* ID of the scan in progress */
short     where;      /* BEFORE or AFTER */
char    *recaddr;     /* adress of the new byte string */
int    length;	      /* length of the new byte string */

/* Inserts 'length' bytes from 'recaddr' to the long data item
      associated with 'scanid'.  If where = AFTER, the data is inserted
      immediately after the scan cursor, otherwise, the data is inserted
      immediatedly before the scan cursor.
  
   RETURNS:
      total number of bytes inserted into long data item
  
   SIDE EFFECTS:
      If necessary, updates all scan cursors (in the scan table)
      of scans on the same long data item
  
   ERRORS:
      e3BADSCANTYPE - if type associated with the scanid is not LONGSCAN
*/
{
    register SCANINFO	*sptr;	    /* address of scan info record */
    register int	insert_count;	/* # of bytes actually inserted */
    int	    cursor;		/* point of insertion */
    SCANINFO	*sptr1;	    /* address of scan info record */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    {
    	printf("am_insertlong(scanid=%d, ", scanid);
    	printf("where=%s", (where == AFTER)? "AFTER" : "BEFORE");
    	printf(", recaddr=0x%x,length=%d)\n", recaddr, length);
    }
#endif
    
    sptr = AM_getscan(scanid);	/* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

    if (sptr->scantype != LONGSCAN)
    	return (e3BADSCANTYPE);	/* this routine is for long scan only */

    if (length <= 0) return(0);

    /* insert into the long data item */
    cursor = (where == AFTER)? sptr->offset + 1 : sptr->offset;
    insert_count = st_insertframe(sptr->filenum, &sptr->rid, cursor,
    	    	recaddr, length, sptr->trans_id, sptr->lockup, sptr->cond);
    CHECKERROR(insert_count);

    /* adjust the cursors of other scans on the same long data item 
       if necessary (ie, all cursors after the cursor of the current 
       scan will be shifted backward)                               */

    for (scanid = AM_nextscan(sptr->filenum, NIL); /* first scan on chain */
    	scanid != NIL; scanid = AM_nextscan(sptr->filenum, scanid))
    {
    	sptr1 = AM_getscan(scanid);
    	if (sptr1 == NULL) return(e3BADSCANID);
    	if (sptr1->scantype != LONGSCAN || !RIDEQ(sptr->rid,sptr1->rid))
    	    continue;	/* not a scan on this item */
    	if (sptr1->offset < cursor)
    	    continue;	/* cursor not affected */
    	if ((sptr1->offset -= insert_count) < cursor)
    	    sptr1->offset = cursor;	/* adjust cursor */
    }

    return (insert_count);	/* return # of bytes inserted */

}  /* am_insertlong */
