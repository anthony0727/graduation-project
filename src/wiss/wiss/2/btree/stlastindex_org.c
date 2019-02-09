
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
/* Module : st_lastindex
  	The routine in this module is st_lastindex. It locates and 
  	returns the RID of the last record whose key (of a particular
  	index) does not exceed the given lower bound.
  
    IMPORTS:
  	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
  	bt_traverse(filenum, key, page@, slotnum@,
  	    parentindex, parent list, trans_id, lockup, oper, cond)
  	bt_rightmost(filenum, rootpid, leafpid, leafpage, trans_id, 
	    lockup, cond);
  	bt_getrid(page, slotnum, offset, rid@, trans_id, lockup, cond);

    EXPORTS:
  	st_lastindex(filenum, UB, Cursor@, LastRID, trans_id, 
		lockup, oper, cond)
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>

  
st_lastindex(filenum, UB, Cursor, LastRID, trans_id, lockup, oper, cond)
int	filenum;	/* open file number */
KEY	*UB;	    /* pointer to the upper bound (may be null) */
XCURSOR	*Cursor;	/* position of the last index found */
RID	*LastRID;	/* where to return the Last RID */
int     trans_id;
short   lockup;
enum    bt_oper    oper;
short   cond;	    /* true if conditional locks are to be used */

/* Returns the RID of the last record (in the order of the index) 
  	whose key does not exceed the given lower bound, described by the
  	structure <KeyType, KeyLen, KeyValue>.  In addition, a B-tree scan
  	cursor, represented as <Page ID, slot #, offset>, is also being set
  	up and returned. The UB may be NULL. In this case, the RID of the 
  	last index is returned.
  
    Returns:
  	RID of the record via parameter
  	the cursor (of an index scan) which points to the index just found 
  
    Side effset:
  	None
  
    Errors:
  	e2NOLASTINDEX : no index satisfy the given UB
*/

{
    int	    e;		/* for returned errors */
    BTREEPAGE	*pageptr;	/* pointer to a page buffer */
    PID	    oldpid, pid;	/* level 1 page id */
    TWO	    slotnum;	/* slot # within the page */
    TWO	    offset;		/* index of the rid within slot */
    PARENTINDEX	parentindex;	/* needed for bttraverse */
    PARENTLIST	parentlist;
    PID             lockpid;        /* A temporary page */
    FID		fid;

#ifdef TRACE
    if (checkset(&Trace2, tINTERFACE)) {
    	printf("st_lastindex(filenum=%d, UB=", filenum);
    	PRINTKEY(UB); printf(", LastRID=0x%x)\n", LastRID);
    }
#endif

    CHECKOFN(filenum);
    fid = FC_FILEID(filenum);

    /* clear the RID first */
    RIDCLEAR(*(LastRID));

    /* get the root page address and file id */ 
    pid = F_ROOTPID(filenum);

    if (UB != NULL) 
    { 
	/* locate the given key first */
    	e = bt_traverse(filenum, UB, &pageptr, &slotnum, &parentindex, 
		parentlist, trans_id, lockup, oper, cond);
    	if (e == e2KEYNOTFOUND) slotnum--;	/* try previous key */
    	else CHECKERROR(e);
    }
    else { /* go down to the rightmost leaf page */
    	e = bt_rightmost(filenum, &pid, &pid, &pageptr, 
		trans_id, lockup, oper, cond);
    	CHECKERROR(e);
    	slotnum = pageptr->btcontrol.numoffsets - 1;
    }

    /* if the slot number is negative, go to the previous page by following
       the prev page pointer, keep trying till a non-empty page found */

    for (pid = pageptr->btcontrol.thispage; slotnum < 0;) 
    {
    	oldpid = pid;    /* keep track of the previous page */
    	/* try next page */
    	pid.Ppage = pageptr->btcontrol.next;

    	/* lock the pages as you move and release the old ones */
    	if (lockup)
    	{
           if (lock_page(trans_id, fid, pid, l_S, MANUAL, cond) == ABORTED) 
	   	return (ABORTED);
	   m_release_page(trans_id, oldpid);
    	}
    	(void) bf_freebuf(filenum, &pageptr->btcontrol.thispage, 
			(PAGE *)pageptr);
    	if (pid.Ppage == NULLPAGE)
    	    break;	/* this is the rightmost page */
    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&pageptr);
    	CHECKERROR(e);
    	slotnum = pageptr->btcontrol.numoffsets - 1;
    }

    if (slotnum >= 0) 
    {
    	MOVERIDCOUNT((char *)&offset, RIDCOUNT(pageptr, slotnum));
    	if (offset < 0) offset = -offset;  /* an overflow rid list */
    }

    /* set up the cursor information */
    if( Cursor != NULL) {
    	Cursor->pageid = pid;
    	Cursor->slotnum = slotnum;
    	Cursor->offset = (--offset);
    }

    if (pid.Ppage == NULLPAGE) e = e2NOLASTINDEX;	/* no index found */
    else 
    { 
	/* return the last RID of this key */
    	e = bt_getrid(filenum, pageptr, slotnum, offset, LastRID, 
		trans_id, lockup, cond);
    	(void) bf_freebuf(filenum, &pageptr->btcontrol.thispage, 
		(PAGE *)pageptr);
    	if (lockup)
    	{
    	    /* lock the data page corresponding to the record */
    	    GETPID(lockpid,*LastRID);
/* DeWitt:  we do not know the fid of the file in which LastRID resides */
/* also, is l_S correct or should the mode depend on the operation */
/* fid is not really correct, nullfid is what should be below */

            if (lock_page(trans_id, fid, lockpid, l_S, MANUAL, cond)
			== ABORTED) return (ABORTED);

/*
      DeWitt,  I deleted the following call because the cursor is pointing
      at this page and it seems incorrect to unlock it.   In fact,
      st_getadjindex seems to depend on it being locked

    	    release the btree leaf page 
    	    m_release_page(trans_id, pid);
*/

    	}
    }

#ifdef TRACE
    if (checkset(&Trace2, tINTERFACE)) {
    	printf("returning from st_lastindex with last RID ");
    	PRINTRIDPTR(LastRID);
    	printf("\n");
    }
#endif
    return(e);

} /* st_lastindex */
