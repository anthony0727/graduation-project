
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


/* Module: bttraverse : Btree traversal to locate the position of a given key.
  
   IMPORTS:
  	bf_readbuf(trans_id, filenum, fid, pageid, returnpage) 
        bf_freebuf(filenum, pageid, pageptr)
  	bt_binary_search(keylen, key, page, slotnum)  from btutil.c
        lock_page (trans_id, file_id, page_id, mode, commit, cond)
  
   EXPORTS:
  	bt_traverse(filenum, key, returnpage, returnslot, pi, pl, trans_id, 
    	lockup, oper, cond)
*/


#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>


extern    int     lock_page();
extern    int     m_release_page();


bt_traverse(filenum, key, returnpage, returnslot, pi, pl,trans_id, lockup, 
    oper, cond)
int	    filenum;	/* open file number of the btree file */
KEY	    *key;    	/* points to key searched for */
BTREEPAGE	**returnpage;   /* leaf page put in *returnpage */
TWO	    *returnslot;	/* slot # on leaf page of key */
PARENTINDEX	*pi;	    /* index of last entry in parent list */ 
PARENTLIST	pl;     	/* list of ancestors (path traversed) */
int             trans_id;
short           lockup;
enum            bt_oper    oper;
short	        cond;  	    /* whether lock is to be conditional */

/* This routine traverses from the root down the tree until the leaf page 
   that contains the key is found. As the search moves down the tree, the 
   path traversed is recorded.
   If the key is found on the leaf page, the slot# is returned in 
   returnslot and eNOERROR is returned.
   If the key is not found, then the value returned in returnslot 
   is the slot of the key which would be directly behind the key 
   if it were on the page and e2KEYNOTFOUND is returned.
  
   RETURNS:
  	returnpage points to the leaf page buffer
  	returnslot gives the position of the entry on the leaf page
  	pl and *pi contains the path to the leaf page 
  
   SIDE EFFECTS:
    the buffer containing the leaf page is fixed in memory
  
   ERRORS:
  	e2KEYNOTFOUND: the key is not found 
*/
{
    int	    e, se;	    /* for returned errors */
    PID	    pid;	/* PID of the current page */
    SHORTPID	nextpid;	/* next page to go */
    TWO	    slotnum;	/* position of the entry on page */
    BTREEPAGE	*btpage;	/* buffer of the current page */
    PARENTINDEX	finger;	    /* index for moving on parent list */ 
    short           clear_locks;    
    FID	    fid;

    /* if flag (clear_locks) is set, all previous locks are cleared */
    
#ifdef TRACE
    if (checkset (&Trace2, tTRAVERSE)) {
    	printf("bt_traverse(filenum=%d,key=", filenum); PRINTKEY(key);
    	printf(",returnpage=0x%x,returnslot=0x%x,",
    	    returnpage, returnslot);
    	printf(",parent_index=0x%x,parent_list=0x%x)\n", pi, pl);
    }
#endif

    fid = FC_FILEID(filenum);

    /* traversal down the btree and record the path on the way */
    pid = F_ROOTPID(filenum);

    for (*pi = 0; ; (*pi)++, pid.Ppage = nextpid) 
    {
    	clear_locks = FALSE;
    	if (lockup)
    	{
       	    switch (oper)
       	    {
    	    case BT_READ :

    	        if ((e = lock_page(trans_id, fid, pid, l_S, MANUAL, cond)) 
			< eNOERROR)
    	        {
		    if (*pi > 0 && pl[((*pi) - 1)].locked == TRUE)
			m_release_page(trans_id, pl[(*pi - 1)].page_id);
    	    	    return (e);
    	        }
    	        pl[*pi].locked = TRUE;
    	        if (*pi > 0)
    	        {
/*
    	    	    e = m_release_page(trans_id, pl[(*pi - 1)].page_id);
*/
    	    	    e = m_release_page(trans_id, pl[((*pi) - 1)].page_id);
    	    	    if (e != OK) 
    	    	  	printf("\nin b-tree traverse, m_release_page returned error %d\n", e);
    	    	    pl[(*pi) - 1].locked = FALSE;
    	        }
    	        break;
    	    case BT_INSERT :
    	        if ((e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond)) 
			< eNOERROR)
    	        {
		    if (*pi > 0 && pl[((*pi) - 1)].locked == TRUE)
			m_release_page(trans_id, pl[(*pi - 1)].page_id);
    	            return (e);
    	        }
    	        pl[*pi].locked = TRUE;
    	        break;
    	    case BT_DELETE :
    	        if ((e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond)) 
			< eNOERROR)
    	        {
		    if (*pi > 0 && pl[((*pi) - 1)].locked == TRUE)
			m_release_page(trans_id, pl[(*pi - 1)].page_id);
    	            return (e);
    	        }
    	        pl[*pi].locked = TRUE;
    	        break;
    	    case BT_INSERT_DELETE:
    	        if ((e = lock_page(trans_id, fid, pid, l_X, MANUAL, cond)) 
			< eNOERROR)
    	        {
		    if (*pi > 0 && pl[((*pi) - 1)].locked == TRUE)
			m_release_page(trans_id, pl[(*pi - 1)].page_id);
    	            return (e);
    	        }
    	        pl[*pi].locked = TRUE;
    	        break;

	    default:
		printf("illegal btree operation=%d in bt_traverse()\n", oper);
    	        return (ABORTED);
       	    }
    	}

	/* now proper lock has been set on the page */

    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **) &btpage);
    	CHECKERROR(e);
    	pl[*pi].page_id = pid;

	/* now that the page has been read, check if we can safely */
	/* release locks on the ancestors in the b-tree */

    	if (lockup)
    	{
       	    switch (oper)
       	    {
    	    case BT_READ :  /* doesn't apply to reads */
    	        break;
    	    case BT_INSERT :
    	        if (OVERFLOW(btpage)) ;
    	        else clear_locks = TRUE;
    	        break;
    	    case BT_DELETE :
    	        if (UNDERFLOW(btpage)) ;
    	        else clear_locks = TRUE;
    	        break;
    	    case BT_INSERT_DELETE:
    	        if ( (UNDERFLOW(btpage)) || (OVERFLOW(btpage)) ) ;
    	        else clear_locks = TRUE;
    	        break;
       	    }
    	}

    	/* search this page for key */
    	se = bt_binary_search(key->length, key->value, btpage, &slotnum);
    	if (btpage->btcontrol.pagetype == LEAFPG)
    	    break;	/* hit the bottom of the tree */

    	/* get the pid where we should go the next level down */
    	if (se == e2KEYNOTFOUND) slotnum--;
    	if (slotnum < 0) 
	{ 
	    /* smaller than any key on page */
    	    nextpid = btpage->btcontrol.pid0;
    	    if (nextpid == NULLPAGE) GETPTR(btpage, 0, nextpid);
    	}
    	else GETPTR(btpage, slotnum, nextpid);

	/* if the locks on previous pages are no longer necessary 
	*  then release them 
	*/
    	if (clear_locks == TRUE)
    	{
    	    for (finger = ( (*pi) - 1); finger >= 0; finger --)
    	    {
    	        if (pl[finger].locked == TRUE)
    	        {
    	            e = m_release_page(trans_id,pl[finger].page_id);
    	            if (e != OK) 
    	                printf("\nin b-tree traverse, m_release_page returned error %d\n", e);
    	            pl[finger].locked = FALSE;
    	        }
    	    }
    	}
    	(void) bf_freebuf(filenum, &pid, (PAGE *)btpage);
    }

    /* now return the buffer address of the leaf page and slot number */
    *returnpage = btpage;
    *returnslot = slotnum;

    return(se);	/* eNOERROR or e2KEYNOTFOUND */

} /* bt_traverse */

