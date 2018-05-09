
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
/* Module : st_getindex
  	The routine in this module is st_getindex. It locates and 
  	returns the RID of the first record whose key (of a particular
  	index) is equal to the given key value.
  
   IMPORTS:
  	bt_traverse(filenum, key, page@, slotnum@, pi, pl, 
		trans_id, lockup, oper, cond)
  	bt_getrid(page, slotnum, offset, rid@, trans_id, lockup, cond);
	clean_locks(pl, pi, trans_id);
  
   EXPORTS:
  	st_getindex(filenumber, SearchKey, Cursor@, FirstRID, 
		trans_id, lockup, oper, cond)
*/

/*
 *  The locking is based on the fact that when the appropriate page is 
 *  found, based on the operation requested the page is locked.
 *  Most probably the operation will always be BT_READ.
 *  The page which CURSOR is set to remains locked even though the buffer
 *  for it is released.
 */

  
#include <wiss.h>
#include <st.h>

st_getindex(filenum, SearchKey, Cursor, FirstRID, trans_id, lockup, oper, cond)
int	filenum;	/* open file number of the index */
KEY	*SearchKey;	/* lower bound for the index key */
XCURSOR	*Cursor;	/* Cursor of the 1st qualified index */
RID	*FirstRID;	/* where to return the qualified RID */
int     trans_id;
short   lockup;
enum    bt_oper    oper;
short	cond;


/* Returns the RID of the first record (in the order of the index) 
  	whose key is equal to the given key value, which is described by the
  	structure <KeyType, KeyLen, KeyValue>.  In addition, a B-tree scan
  	cursor, represented by <Page ID, slot #, offset>, is also being set
  	up and returned. The SearchKey may be NULL. In this case, the RID 
        of the first index is returned.
  
    Returns:
  	the RID of the first qualified record 
  	the cursor (of an index scan) which points to the index just found 
  					
    Side effects:
  	None
  
    Errors:
  	e2KEYNOTFOUND: no records matches the key
*/

{
	int		e;		/* for returned errors */
	BTREEPAGE	*pageptr;	/* page buffer pointer */
	PID		pid;		/* level 0 page id */
	TWO		slotnum;	/* slot # within the page */
	PARENTINDEX	pi;		/* needed for bttraverse */
	PARENTLIST	pl;		/* parent stack */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_getindex(filenum=%d, SearchKey=", filenum);
		PRINTKEY(SearchKey);
		printf(" , *Cursor=0x%x, *FirstRID=0x%x)\n", Cursor, FirstRID);
	}
#endif

	CHECKOFN(filenum);

	/* clear the RID first */
	RIDCLEAR(*(FirstRID));

	if (SearchKey == NULL) return(e2KEYNOTFOUND);

	/* get the file id and the page address of the root */
	pid = F_ROOTPID(filenum);

	/* locate the given key first */
	e = bt_traverse(filenum, SearchKey, &pageptr, &slotnum, &pi, pl, 
		trans_id, lockup, oper, cond);
	if (e < eNOERROR) 
	{
		(void) bf_freebuf(filenum, &pl[pi].page_id, (PAGE *)pageptr);
/*
 *  if lockup was true then release all the locks before returning error.
 */
		if (lockup) clean_locks(pl, pi, trans_id);
		return(e2KEYNOTFOUND);
	}

	/* prepare the cursor position of the first index */
	if (Cursor != NULL) {
		Cursor->pageid = pageptr->btcontrol.thispage;
		Cursor->slotnum = slotnum;
		Cursor->offset = 0;
	}

	/* get the first RID the key we just located */
	e = bt_getrid(filenum, pageptr, slotnum, 0, FirstRID, trans_id,
		lockup, cond);

/*
 *  SHERROR, This could be an error, since all the pages beside the leaf
 *  pages are released.
 */
	if (lockup) clean_locks(pl, (pi - 1), trans_id);
	(void) bf_freebuf(filenum, &pl[pi].page_id, (PAGE *)pageptr);
#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("returning from st_getindex with first RID = ");
		PRINTRIDPTR(FirstRID);
		printf("\n");
	}
#endif
	return(e);

} /* st_getindex */
