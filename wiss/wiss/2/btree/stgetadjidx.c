
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
/* Module : st_getadjrid
   	Given a cursor of an index scan, return the RID associated
  	with either the next or the previous index 
  
    IMPORTS:
  	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
  	bf_setdirty(filenum, pageid, pageptr)
  	bt_getrid(page, slotnum, offset, rid@, trans_id, lockup, cond);
  
    EXPORTS:
  	st_getadjrid(filenum, Cursor@, which, AdjRID@, trans_id, 
		lockup, oper, cond)
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>

st_getadjrid(filenum, which, Cursor, AdjRID, trans_id, lockup, oper, cond)
int	filenum;	/* where to work on */
int	which;		/* PREV or NEXT */
XCURSOR	*Cursor;	/* position of the current index in the btree */
RID	*AdjRID;	/* where to return the adjacent RID */
int     trans_id;
short   lockup;         /* if lockup is TRUE, lock the page in desired mode */
enum    bt_oper    oper;/* the operation which decides the lock mode */
short	cond;

/* Given a cursor of an index scan, return either the next or the 
  	previous RID of the same key. If the RID list is exhausted, then
  	proceed to the next or the previous key.
  	A curosr is represented by <page ID, slot #, offset>.
  	'Which' should be set to NEXT or PREV.
  
    Returns:
   	Returns via AdjRID the RID of either the next or the previous RID
  
    Side effects:
  	none
  
    Errors:
   	e2NONEXTTRID : If there is no next RID
   	e2NOPREVTRID : If there is no previous RID 
*/

{
    int		e;		/* for returned errors */
    BTREEPAGE	*leaf;		/* the leaf page been worked on */
    PID		pid;		/* level 0 page id */
    TWO		ridcount;	/* # of RIDs belongs to this key */ 
    TWO		offset;		/* index of the member rid */
    TWO		slotnum;	/* index within the leaf page */
    FID		fid;
    int		releaseFlag;
    PID		releasePid;
    
#ifdef TRACE
    if (checkset(&Trace2, tINTERFACE))
    	printf("st_getadjrid(filenum=%d,which=%d)\n",
    		filenum, which, Cursor, AdjRID);
#endif


    CHECKOFN(filenum);
    fid = FC_FILEID(filenum);
/*
 *  ASSUMPTION : It is persumed that the routine which is calling this
 *  routine has already done some sort of traversal, and has already locked
 *  the leaf and corresponding pages according to the oper.
 */

    /* get the information in cursor */
    pid = Cursor->pageid;
    slotnum = Cursor->slotnum; 
    offset = Cursor->offset; 
    if (pid.Ppage == NULLPAGE) 
    {
    	return(e2ILLEGALCURSOR);
    }

    /* find the page the cursor is on and read it in */
    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, (PAGE **)&leaf);
    CHECKERROR(e);

    /* check the validity of the cursor */
    MOVERIDCOUNT((char *)&ridcount, RIDCOUNT(leaf, slotnum));
    if (ridcount < 0) ridcount = -ridcount;	/* long RID list */
    if (slotnum > leaf->btcontrol.numoffsets || 
    		offset < 0 || offset >= ridcount) {
    	(void) bf_freebuf(filenum, &pid, (PAGE *)leaf);
/*
    	printf("st_getadjrid().  Illegal cursor #2 %d %d %d \n",
		pid.Ppage, slotnum, offset);
*/
    	return(e2ILLEGALCURSOR);
    }

    if (which == PREV) { /* back up the cursor and get the previous rid */
    	if ((--offset) < 0) {
    	    /* if the slot number is negative, go to the previous 
    	       page, keep trying till a non-empty page found */
    	    for (--slotnum; slotnum < 0;) {
    	    	/* try the previous page */
    	    	if((pid.Ppage=leaf->btcontrol.prev) == NULLPAGE)
    	    	    break;	/* this is the rightmost page */
    	    	(void) bf_freebuf(filenum, &leaf->btcontrol.thispage, 
			(PAGE *)leaf);
    	    	if (lockup == TRUE)
    	    	{
    	    	    switch (oper)
    	    	    {
    	    	    case BT_READ :
		        if ((e = lock_page(trans_id, fid, pid, l_S, MANUAL, 
				cond, TRUE)) < eNOERROR) return (e);
    	    	        break;
    	    	    case BT_INSERT :
    	    	    case BT_DELETE :
    	    	    case BT_INSERT_DELETE :
		        if ((e = lock_page(trans_id, fid, pid, l_X, MANUAL, 
				cond, TRUE)) < eNOERROR) return (e);
    	    	        break;
    	    	    }
    	    	}
    	    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
			(PAGE **)&leaf);
    	    	CHECKERROR(e);
    	    	slotnum = leaf->btcontrol.numoffsets - 1;
    	    	if ((slotnum < 0) && (lockup == TRUE))
    	    	    m_release_page (trans_id, pid);
    	    }
    	    if (pid.Ppage != NULLPAGE) { /* find the prevous RID */
    	    	MOVERIDCOUNT((char *)&offset, 
    	    	    RIDCOUNT(leaf, slotnum));
    	    	if (offset < 0) offset = -offset;
    	    	offset--; /* go to the prev key */
    	    }
    	}
    }
    else 
    {	
	 /* move the cursor forward and get the next rid */
    	if ((++offset) >= ridcount) 
	{
    	    /* if the slot number is too large, go to the next 
    	       page, keep trying till a non-empty page found */
    	    if (++slotnum >= leaf->btcontrol.numoffsets)
    	    while(TRUE) {
    	    	if((pid.Ppage=leaf->btcontrol.next) == NULLPAGE)
    	    	    break;	/* this is the rightmost page */

    	    	(void) bf_freebuf(filenum, 
    	    	    &leaf->btcontrol.thispage, (PAGE *)leaf);
    	    	if (lockup == TRUE)
    	    	{
    	    	    switch (oper)
    	    	    {
    	    	    case BT_READ :
		        if ((e = lock_page(trans_id, fid, pid, l_S, MANUAL, 
				cond, TRUE)) < eNOERROR) return (e);
    	    	        break;
    	    	    case BT_INSERT :
    	    	    case BT_DELETE :
    	    	    case BT_INSERT_DELETE :
		        if ((e = lock_page(trans_id, fid, pid, l_X, MANUAL, 
				cond, TRUE)) < eNOERROR) return (e);
    	    	        break;
    	    	    }
    	    	}
    	    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
			(PAGE **)&leaf);
    	    	CHECKERROR(e);
    	    	if (leaf->btcontrol.numoffsets != 0) {
    	    	/* get the 1st key on the non-empty page */
    	    	    slotnum = 0;
    	    	    break;
    	    	}
    	    	else
    	    	{
    	    	    if (lockup)
    	    	    m_release_page(trans_id, pid);
    	    	}
    	    }
    	    offset = 0;	/* first RID of the next key */

    	} /* end of offset too large */
    }

    /* 
     * if the cursor does not point to the same page then it means that you have
     * to free the previous page.  Save the necessary information and do it at 
     * the very end when we are positive the page has been unfixed 
     */
    if ((Cursor->pageid.Pvolid != pid.Pvolid) || 
     (Cursor->pageid.Ppage != pid.Ppage))
    {
	releaseFlag = TRUE;
	releasePid = Cursor->pageid;
    }
    else releaseFlag = FALSE;


    /* update the cursor */
    if (pid.Ppage == NULLPAGE) offset = slotnum = -1;
    Cursor->offset = offset;
    Cursor->slotnum = slotnum;
    Cursor->pageid = pid;

    if (pid.Ppage == NULLPAGE)
    	e = (which == PREV ? e2NOPREVRID : e2NONEXTRID);
    else /* return the RID via AdjRID */
    	e = bt_getrid(filenum, leaf, slotnum, offset, AdjRID, 
		trans_id, lockup, cond);

    (void) bf_freebuf(filenum, &leaf->btcontrol.thispage, (PAGE *)leaf);

#ifdef TRACE
    if (checkset(&Trace2, tINTERFACE)) {
    	printf("returning from st_getadjrid ");
    	if (e >= eNOERROR) {
    	    printf(" with adjacent RID = ");
    	    PRINTRIDPTR(AdjRID); printf("\n");
    	}
    }
#endif
    /* at this point it is always safe to do the release */
    if (releaseFlag) m_release_page(trans_id, releasePid);

    return(e);

} /* st_getadjrid */
