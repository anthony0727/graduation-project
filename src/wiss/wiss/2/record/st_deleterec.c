
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
/* Module st_deleterecord : delete a record

   IMPORTS :
	bf_setdirty(filenum, pageid, pageptr)
	bf_freebuf(filenum, pageid, pageptr)
	r_slide(page, slot, newlength, recptr, trans_id, lockup)
	r_dropout(filenum, pageid, trans_id, lockup)
	r_getslot(filenum, ridptr, pageptr, recptr, trans_id, lockup, mode, cond)

   EXPORTS :
	st_deleterecord(filenum, ridptr, trans_id, lockup, cond)

*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

st_deleterecord(filenum, ridptr, trans_id, lockup, cond)
int	filenum;	/* index into file tab's open file table */
RID	*ridptr;	/* rid that gets the ax */
int     trans_id;
short   lockup; 
short	cond;

/* This procedure deletes a record from a file.
   If the record has been moved to a new location, both the record 
   and its forwarding address are deleted.

   Returns:
	None

   Side Effects:
	pages of the file may be deallocated 

   Errors:
	e2NULLRIDPTR: no RID
	e2DELNOEXTREC: attempt to delete a non-existing record

*/
{
	int		e; 	/* for returned errors */
	int		ridcnt;	/* how many records left on the page */
	RID		newaddr;/* the forwarding address of a moved record */
	PID		mypid, pid;    /* which page the record is on */
	DATAPAGE	*dp;	/* buffer page that contains the rid */
	RECORD		*recptr;/* record pointer */

#ifdef TRACE
	if (checkset(&Trace2,tINTERFACE)) {
		printf("st_deleterecord(filenum = %d,",filenum);
		printf("ridptr = "); PRINTRIDPTR(ridptr); printf(")\n");
	}
#endif

	/* check input parameters */
	CHECKOFN(filenum);
	CHECKWP(filenum);
	GETPID (mypid, *ridptr);
	if (ridptr == NULL) return(e2NULLRIDPTR);
	if (TESTRIDCLEAR(*ridptr)) return(e2DELNOEXTREC);

	if (lockup)
	{
	    e = lock_page (trans_id, FC_FILEID(filenum), mypid, l_X, COMMIT, 
		cond);
	    CHECKERROR(e);
	}

	/* read the page in, and remove the record */
	/* lockup is passed as FALSE since the page has been locked */
	e = r_getslot(filenum, ridptr, &dp, &recptr, trans_id, FALSE, l_NL, cond);
	CHECKERROR(e);
	if (recptr->type == MOVED) 
	{ 	/* get the fordwarding address */
		newaddr = * ((RID *) recptr->data);
		GETPID (mypid, newaddr);
		if (lockup) {
	        	e = lock_page (trans_id, FC_FILEID(filenum), mypid, 
				l_X, COMMIT, cond);
	        	CHECKERROR(e);
		}
	}
	else newaddr.Rpage = NULLPAGE;

	/* lockup = FALSE is passed to r_slide since page has already been locked */
	(void) r_slide(dp, ridptr->Rslot, REMOVEREC, &recptr, trans_id, 
		FALSE,cond);
	ridcnt = dp->ridcnt;	/* the # of records left on the page */
	(void) bf_setdirty(filenum, &(dp->thispage), dp);
	(void) bf_freebuf(filenum, &(dp->thispage), dp);

	/* release the page if it has become empty */
	if (ridcnt == 0) { 
		GETPID(pid, *ridptr);
		e = r_dropout(filenum, &pid, trans_id, FALSE, cond);
		CHECKERROR(e);
	}

	/* if the record has been moved, track it down */
	if (newaddr.Rpage != NULLPAGE) 
	{
	    e = r_getslot(filenum, &newaddr, &dp, &recptr, trans_id, FALSE, 
			l_NL, cond);
	    CHECKERROR(e);
	    (void) r_slide(dp, newaddr.Rslot, REMOVEREC, &recptr, trans_id, 
			FALSE, cond);
	    ridcnt = dp->ridcnt;
	    (void) bf_setdirty(filenum, &(dp->thispage), dp);
	    (void) bf_freebuf(filenum, &(dp->thispage), dp);

	    /* release the page if it has become empty */
	    if (ridcnt == 0) 
	    { 
		GETPID(pid, newaddr);
		e = r_dropout(filenum, &pid, trans_id, FALSE, cond);
		CHECKERROR(e);
	    }
	}
		
	if (F_FILETYPE(filenum) == DATAFILE)
	{
		F_CARD(filenum)--, F_STATUS(filenum) = DIRTY; 
	}

	return(eNOERROR);

} /* st_deleterecord */

