
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


/* Module : Btree Utility Module (*)
      This module contains routines that touch a significant part of a B-tree
    Some of the routines correspond to the concurrency part
  
   IMPORTS:
      bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
      bf_freebuf(filenum, pageid, pageptr)
  
   EXPORTS:
      bt_leftmost(filenum, RootPID, LeftMostPid, LeftMostPage, 
    	trans_id, lockup, cond)
      bt_rightmost(filenum, RootPID, RightMostPid, RightMostPage, 
    	trans_id, lockup, cond)
      int bt_prefixkey(filenum, page, slot_num, key_length, 
    	key_value, trans_id, lockup, cond)
    clean_locks (pl, pi, trans_id)
  
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>


clean_locks (pl, pi, trans_id)
PARENTLIST        pl;
PARENTINDEX       pi;
int    	trans_id;
{
    PARENTINDEX    finger;

    for (finger = pi; finger >= 0; finger --)
    {
    	if (pl[finger].locked == TRUE)
    	{
/*
             printf ("%d Trying to release page(%d, %d), finger %d.\n",
         	trans_id, pl[finger].page_id.Pvolid, 
         	pl[finger].page_id.Ppage, finger);
*/
            m_release_page(trans_id, pl[finger].page_id);
            pl[finger].locked = FALSE;
    	}
    }
}


bt_leftmost(filenum, RootPid, LeftMostPid, LeftMostPage, 
    trans_id, lockup, oper, cond)
int    	filenum;
PID    	*RootPid;	/* pid of the root of this subtree */
PID    	*LeftMostPid;	/* pid of the left most page */
BTREEPAGE    **LeftMostPage;	/* buffer pointer of the left most page */
int             trans_id;
short           lockup;
enum            bt_oper     oper;
short           cond;

/* Given the root of a subtree, return the PID and a buffer pointer
      of its leftmost leaf page.
  
    Returns:
      the PID of the leftmost leaf page in the subtree 
      the buffer pointer of the leaf page

    Side effects:
    buffer of the page is fixed in memory
  
    Errors:
      None
*/
/*
 *  SHERROR
 *  One source of error could be if an updater decides to call bt_leftmost,
 *  the problem is that the pages are locked in shared mode.
 */

/* DeWitt 1/30/90 - this comment now looks incorrect as the switch statement on
 oper seems to set the correct mode of lock */

{
    int	    e;	    /* for returned errors */
    PID	    oldpid, pid;	/* level 0 page id */
    SHORTPID	pid0;	    /* the leftmost page pointer */
    BTREEPAGE	*btpage;	/* page buffer pointer */
    FID	    fid;

#ifdef TRACE
    if (checkset(&Trace2, tBTREEUTIL)) {
    	printf("bt_leftmost(filenum=", filenum); 
    	printf(", RootPID="); PRINTPIDPTR(RootPid);
    	printf(", *LeftMostPid=0x%x, **LeftMostPage=0x%x)\n", 
    	    LeftMostPid, LeftMostPage);
    }
#endif
    
    fid = FC_FILEID(filenum);

    PIDCLEAR (oldpid);
    for (pid = *RootPid; pid.Ppage != NULLPAGE; pid.Ppage = pid0) 
    {
    	if (lockup == TRUE)
    	{
	    /* modified by PHJ */
    	    if (TESTPIDCLEAR(oldpid));
    	    else m_release_page(trans_id, oldpid);
    	    switch(oper)
    	    {
    	    case BT_READ :
    	        if ((e = lock_page(trans_id, fid, pid, l_S, MANUAL, cond)) 
			< eNOERROR)
    	    		return (e);
    	        break;
    	    case BT_INSERT :
    	    case BT_DELETE :
    	    case BT_INSERT_DELETE :
    	        if ((e = lock_page (trans_id, fid, pid, l_X, MANUAL, cond)) <
			eNOERROR)
    	            return (e);
    	        break;
    	    }
    	}
    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&btpage);
    	CHECKERROR(e);
    	if (btpage->btcontrol.pagetype == LEAFPG)
    	    break; 	/* reach the bottom of the tree */
    	pid0 = btpage->btcontrol.pid0;
    	if (pid0 == NULLPAGE) GETPTR(btpage, 0, pid0);
    	(void) bf_freebuf(filenum, &pid, (PAGE *)btpage);
    	oldpid = pid;
    }

    /* return the pid and page buffer pointer */
    *LeftMostPid = pid;
    *LeftMostPage = btpage;

#ifdef TRACE
    if (checkset(&Trace2, tBTREEUTIL))
    	printf("return from bt_leftmost\n"); 
#endif

    return(eNOERROR);

} /* bt_leftmost */


bt_rightmost(filenum, RootPid, RightMostPid, RightMostPage,
	trans_id, lockup, oper, cond)
int    	filenum;	/* file number */
PID    	*RootPid;	/* pid of the root of this subtree */
PID    	*RightMostPid;	/* pid of the right most page */
BTREEPAGE    **RightMostPage; /* buffer pointer of the right most page */
int             trans_id;
short           lockup;
enum            bt_oper    oper;
short           cond;

/* Given the root of a subtree, return the PID and the
      buffer of its rightmost leaf page.
  
    Returns:
      the PID of the rightmost leaf page in the subtree
      the buffer pointer of the leaf page
      	    	    
    Side effects:
    buffer of the page is fixed in memory
  
    Errors:
      None
*/

{
    int	    e;	    /* for returned errors */
    PID	    oldpid, pid;	/* level 0 page id */
    SHORTPID	lpid;	    /* leftmost pointer */
    BTREEPAGE	*pageptr;	/* page buffer pointer */
    FID		fid;

#ifdef TRACE
    if (checkset(&Trace2, tBTREEUTIL)) {
    	printf("bt_rightmost(filenum =", filenum);
    	printf(", RootPID="); PRINTPIDPTR(RootPid);
    	printf(", *RightMostPid=0x%x, **RightMostPage=0x%x)\n", 
    	    RightMostPid, RightMostPage);
    }
#endif

    PIDCLEAR(oldpid);
    fid = FC_FILEID(filenum);
    for (pid = *RootPid; pid.Ppage != NULLPAGE; pid.Ppage = lpid) 
    {
    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&pageptr);
    	CHECKERROR(e);
/*
 *  Notice that the pages are locked in shared or exclusive mode.
 *  As soon as one of the pages is locked the previous one is released,
 *  the ultimate affect is that the appropriate leaf page is locked in 
 *  Shared mode.
 */
    if (lockup == TRUE)
    {
	/* modified by PHJ */
    	if (TESTPIDCLEAR(oldpid));
    	else m_release_page(trans_id, oldpid);
        switch(oper)
        {
    	    case BT_READ :
    	        if ((e = lock_page (trans_id, fid, pid, l_S, MANUAL, cond)) 
			< eNOERROR)
    	    		return (e);
    	        break;
    	    case BT_INSERT :
    	    case BT_DELETE :
    	    case BT_INSERT_DELETE :
    	        if ((e = lock_page (trans_id, fid, pid, l_X, MANUAL, cond)) 
			< eNOERROR)
    	    		return (e);
    	        break;
    	    }
    	}
    	if (pageptr->btcontrol.pagetype == LEAFPG)
    	    break; 	/* reach the bottom of the tree */
    	if (pageptr->btcontrol.numoffsets == 0)
    	    lpid = pageptr->btcontrol.pid0;
    	else GETPTR(pageptr, pageptr->btcontrol.numoffsets-1, lpid);
    	(void) bf_freebuf(filenum, &pid, (PAGE *)pageptr);
    	oldpid = pid;
    }

    /* return the pid and page buffer pointer */
    *RightMostPid = pid;
    *RightMostPage = pageptr;

#ifdef TRACE
    if (checkset(&Trace2, tBTREEUTIL))
    	printf("return from bt_rightmost\n");
#endif

    return(eNOERROR);

} /* bt_rightmost */


int
bt_prefixkey(filenum, bt_page, slot_num, k_len, k_val, trans_id, lockup, cond)
int    	filenum;	/* file number */
BTREEPAGE    *bt_page;	/* on which to put the key */
TWO    	slot_num;	/* which slot in the page to put the key **/
TWO    	k_len;	    /* length of the key */
char    	*k_val;	    /* value of the key (must be string) */
int             trans_id;
short           lockup;
short           cond;

/* Given a key and a page on which to insert it, find the shortest
      prefix that is sufficient to distinguish that key from the largest
      key in the subtree of its left neighbor.
      This routine should be called only for keys of string type.
  
    Returns:
      length of the prefix key

    Side effects:
      None
  
    Errors:
      None
*/
{
    int	    e;	    /* for returned errors */
    TWO	    j;
    BTREEPAGE	*lp;	    /* right most leaf of left neighbor */
    char	    *leftkey;	/* last key on the proceeding page */
    PID	    lpid;	    /* page id of the left neighbor */
    PID	    pid;	    /* page id of page the key should go */

#ifdef TRACE
    if (checkset(&Trace2, tBTREEUTIL))
    	printf("bt_prefixkey(filenum=0x%x,page=0x%x,slot=%d,key(%d)=%.*s)\n",
    	    bt_page, slot_num, k_len, k_len, k_val);
#endif

    /* get the pointer on the left */
    pid = bt_page->btcontrol.thispage;
    lpid.Pvolid = pid.Pvolid;
    if (slot_num > 0) GETPTR(bt_page, slot_num-1, lpid.Ppage);
    else lpid.Ppage = bt_page->btcontrol.pid0;

    /* get the rightmost page of this subtree and then the last key on it */
    e = bt_rightmost(filenum, &lpid, &lpid, &lp, trans_id, 
	lockup, BT_READ, cond);
    CHECKERROR(e);
    j = lp->btcontrol.numoffsets - 1;
    leftkey = KEYVALUE(lp, j);

    /* find the prefix key */
    j = MIN(k_len, KEYLEN(lp, j));
    for (k_len=1; k_len<=j; k_len++) 
    	if (*k_val++ != *leftkey++) break; 	/* enough ! */

    /* free that rightmost page */
    (void) bf_freebuf(filenum, &lpid, (PAGE *)lp);

    /* CC.. Free the right most page which was just read */
    if (lockup) m_release_page(trans_id, lpid);

#ifdef TRACE
    if (checkset(&Trace2, tBTREEUTIL))
    	printf("returning from bt_prefixkey, new length=%d\n", k_len);
#endif

    return(k_len);

} /* bt_prefixkey */

