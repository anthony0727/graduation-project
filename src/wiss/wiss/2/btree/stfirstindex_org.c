
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


/* Module : st_firstindex
  	The routine in this module is st_firstindex. It locates and 
  	returns the RID of the first record whose key (of a particular
  	index) does not precede the given lower bound.
  
   IMPORTS:
  	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
  	bt_traverse(filenum, key, page@, slotnum@, parentindex, 
	  	parent list, trans_id, lockup, oper, cond)
  	bt_leftmost(filenum, rootpid, leafpid@, leafpage@, trans_id, 
		lockup, oper, cond);
  	bt_getrid(page, slotnum, offset, rid@, trans_id, lockup, cond);
  
   EXPORTS:
  	st_firstindex(filenumber, LB, Cursor@, FirstRID)
*/
  
#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>

st_firstindex(filenum, LB, Cursor, FirstRID, trans_id, lockup, oper, cond)
int	filenum;	/* open file number of the index */
KEY	*LB;		/* lower bound for the index key */
XCURSOR	*Cursor;	/* Cursor of the 1st qualified index */
RID	*FirstRID;	/* where to return the qualified RID */
int     trans_id;
short   lockup;
enum    bt_oper     oper;
short   cond;


/* Returns the RID of the first record (in the order of the index) 
  	whose key does not precede the given lower bound, described by the
  	structure <KeyType, KeyLen, KeyValue>.  In addition, a B-tree scan
  	cursor, represented by <Page ID, slot #, offset>, is also being set
  	up and returned. The LB may be NULL. In this case, the RID of the 
  	first index is returned.
  
    Returns:
  	the RID of the first qualified record 
  	the cursor (of an index scan) which points to the index just found 
  					
    Side effects:
  	None
  
    Errors:
  	e2NOFIRSTINDEX : no records that satisfy UB are found
*/

{
	int		e;		/* for returned errors */
	BTREEPAGE	*pageptr;	/* page buffer pointer */
	PID		oldpid, pid;	/* level 0 page id */
	SHORTPID	nextpid;	/* where to go next */
	TWO		slotnum;	/* slot # within the page */
	PARENTINDEX	parentindex;	/* needed for bttraverse */
	PARENTLIST	parentlist;	/* parent stack */
	PID             lockpid;        /* A temporary page */
	FID		fid;

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_firstindex(filenum=%d, LB=", filenum);
		PRINTKEY(LB);
		printf(" , *Cursor=0x%x, *FirstRID=0x%x)\n", Cursor, FirstRID);
	}
#endif
	CHECKOFN(filenum);

	fid = FC_FILEID(filenum);

	/* clear the RID first */
	RIDCLEAR(*(FirstRID));

	/* locate the key */
	pid = F_ROOTPID(filenum);
	if (LB != NULL) { /* locate the given key first */
		e = bt_traverse(filenum, LB, 
			&pageptr, &slotnum, &parentindex, parentlist, trans_id,
			lockup, oper, cond);
		if (e != e2KEYNOTFOUND && e < eNOERROR)
			return(e);	/* something strange happened */
	}
	else { /* go down to the leftmost leaf page */
		e = bt_leftmost(filenum, &pid, &pid, &pageptr, 
			trans_id, lockup, oper, cond);
		CHECKERROR(e);
		slotnum = 0; /* get the 1st key on the first non-empty page */
	}

	/* 
	 * if the slot number is too large, go to the next page by following 
	 * the next page pointer, keep trying till a non-empty page found. 
	 * Note that bt_traverse in the above has already locked the pid in 
	 * shared mode.  Same is true with bt_leftmost which has locked the
	 * pid in shared mode.
	 */
	for (pid = pageptr->btcontrol.thispage;
		slotnum >= pageptr->btcontrol.numoffsets; ) 
	{
	    nextpid = pageptr->btcontrol.next;
	    (void) bf_freebuf(filenum, &pid, (PAGE *)pageptr);
	    oldpid = pid;
	    pid.Ppage = nextpid;

/*
 *  As you are moving horizontally on the leaf pages of one level you
 *  should lock each page in Shared or eXclusive mode since you dont
 *  want to catch an instance of update of another transaction.
 *  Note that the previous empty pages are released.  The page which
 *  contains the first record remains in the requested operation.
 */
	    if (lockup == TRUE)
	    {
	        switch (oper)
	        {
	            case BT_READ : 
		        if (lock_page(trans_id, fid, pid, l_S, MANUAL, 
			    cond) == ABORTED) return (ABORTED);
		        m_release_page(trans_id, oldpid);
			break;
		    case BT_INSERT :
		    case BT_DELETE :
		    case BT_INSERT_DELETE :
		        if (lock_page(trans_id, fid, pid, l_X, MANUAL, 
				cond) == ABORTED) return (ABORTED);
		        m_release_page(trans_id, oldpid);
		        break;
		}
	    }

	    if (nextpid == NULLPAGE) break;	/* reached the rightmost page */
	    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&pageptr);
	    CHECKERROR(e);
	    if (pageptr->btcontrol.numoffsets != 0) 
	    {
		/* a non-empty page found, get its 1st key as our result */
		slotnum = 0;
		break;
	    }
	}
	
	/* return the cursor position of the first index */
	if( Cursor != NULL) {
		Cursor->pageid = pid;
		Cursor->slotnum = slotnum;
		Cursor->offset = 0;
	}

	if (pid.Ppage == NULLPAGE) e = e2NOFIRSTINDEX;	/* no qualified index found */
	else 
	{   /* get the first RID the key we just located */
	    e = bt_getrid(filenum, pageptr, slotnum, 0, FirstRID, 
		trans_id, lockup, cond);
	    (void) bf_freebuf(filenum, &pid, (PAGE *)pageptr);

	    if (lockup)
	    {
	        /* lock the data page corresponding to the record */
	        GETPID(lockpid,*FirstRID);

/* DeWitt:  is S really safe below or should it depend on the operation */
/* DeWitt:  note, we don't know the fid of the file in which FirstRID resides */
/* DeWItt:  NullFID is used since the fid is not known */
/*
	I deleted the following call in the systemV version of
	wiss, because it really is not correct to lock a page with
	a nullfid.  the o2 version of wiss did support this form of
	locking (ie. a NullFID) in order to handle objects referencing
	other objects without knowing what file they were in.

		if (lock_page(trans_id, NullFID, lockpid, l_S, MANUAL, 
			cond) == ABORTED) return (ABORTED);
*/
	    }
	}
		
#ifdef TRACE
	if (checkset(&Trace2, tBTREE)) {
		printf("returning from st_firstindex with first RID = ");
		PRINTRIDPTR(FirstRID);
		printf("\n");
	}
#endif

	return(e);

} /* st_firstindex */
