
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
/* Module : st_nexthash
   	Given a cursor of a hash scan, return the RID associated
  	with either the next or the previous index 
  
    IMPORTS:
  	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
  	bf_setdirty(filenum, pageid, pageptr)
  	bt_getrid(filenum, page, slotnum, offset, ridptr, trans_id, lockup,
		cond)
  
    EXPORTS:
  	st_nexthash(filenum, Cursorptr, NextRIDptr, trans_id, lockup,
		cond)
*/

#include <wiss.h>
#include <st.h>
#include <lockquiz.h>
  
st_nexthash(filenum, Cursor, NextRID, trans_id, lockup, cond)
int	filenum;	/* where to work on */
XCURSOR	*Cursor;	/* position of the current index in the hash file */
RID	*NextRID;	/* where to return the adjacent RID */
int	trans_id;
short	lockup;
short	cond;

/* Given a cursor of a hash scan, return the next RID of the same key. 
  
    Returns:
   	Returns via NextRID the RID of the next record
  
    Side effects:
  	none
  
    Errors:
   	e2NONEXTTRID: If there is no next RID
	e2ILLEGALCURSOR: a bad cursor
*/
{
	int		e, er;		/* for returned errors */
	BTREEPAGE	*dp;		/* the leaf page been worked on */
	PID		pid;		/* level 0 page id */
	TWO		ridcount;	/* # of RIDs belongs to this key */ 
	TWO		offset;		/* index of the member rid */
	TWO		slotnum;	/* index within the leaf page */

#ifdef TRACE
	if ( checkset(&Trace2,tINTERFACE) ) {
		printf("st_nexthash(filenum=%d, cursor=%d:%d:%d", filenum, 
			Cursor->pageid.Ppage, Cursor->slotnum, Cursor->offset);
		printf(", ridptr=0x%x)\n", NextRID); 
	}
#endif
	
	CHECKOFN(filenum);

	/* get the information in cursor */
	pid = Cursor->pageid;
	slotnum = Cursor->slotnum;
	offset = Cursor->offset;

	if (pid.Ppage == NULLPAGE) return(e2ILLEGALCURSOR);

	/* find the page the cursor is on and read it in */
	/* 
	 * this page has been already locked by st_gethash. 
	 */

	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&dp);
	CHECKERROR(e);

	/* check the validity of the cursor */
	MOVERIDCOUNT((char *)&ridcount, RIDCOUNT(dp, slotnum));
	if (ridcount < 0) ridcount = -ridcount;	/* long RID list */
	if (slotnum>dp->btcontrol.numoffsets || offset<0 || offset>=ridcount) 
	{
		e = e2ILLEGALCURSOR;
		goto error;
	}
	if (++offset == ridcount) 
	{
            /* release lock on the page as there are no more records
	    with this key */
	    (void) bf_freebuf(filenum, &pid, (PAGE *) dp);
	    if (lockup) (void) m_release_page(trans_id, pid);
	    return(e2NONEXTRID);
	}

	/* advance the cursor */
	Cursor->offset++;

	/* get the next RID of this key */
	e = bt_getrid(filenum, dp, slotnum, offset, NextRID, trans_id, lockup,
		cond);
error:
	(void) bf_freebuf(filenum, &pid, (PAGE *) dp);
	return(e);

} /* st_nexthash */
