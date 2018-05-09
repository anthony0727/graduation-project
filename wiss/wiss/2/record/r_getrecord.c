
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
/* Module r_getrecord : Level 2 internal routine to locate a record

   IMPORTS:
        bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
	bf_freebuf(filenum, pageid, pagebuf)
	bf_setdirty(filenum, pageid, pagebuf)

   EXPORTS:
	r_getslot(filenum, ridptr, pageptr, recptr, trans_id, 
		lockup, mode, cond)
	r_getrecord(filenum, ridptr, pageptr, recptr, trans_id, 
		lockup, mode, cond)
*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


r_getslot(filenum, ridptr, pageptr, recptr, trans_id, lockup, mode, cond)
int		filenum;	/* open file number */
RID		*ridptr;	/* *RID */
DATAPAGE	**pageptr;	/* out parameter - pointer to page buffer */
RECORD		**recptr;	/* out parameter - record address */
int             trans_id;       
short           lockup;        
LOCKTYPE	mode;
short           cond;   

/* Given a RID, locate the record and return its address 
   This routine does not care whether a record has been moved or not.

   Returns:
	record address (via recptr)
	page address (via pageptr) 

   Side Effects:
	fix the buffer which holds the record

   Errors:
	e2BADSLOTNUMBER - invalid slot number
*/
{
	int		e;	/* for returned errors */
	PID		pid;	/* the page the record is on */
	int		slotno;	/* which slot on the page */
	DATAPAGE	*dp;	/* pointer to the page buffer */
	
#ifdef TRACE
	if (checkset(&Trace2, tGETRECORD)) {
		printf("r_getslot(filenum=%d%, RID=", filenum);
		PRINTRIDPTR(ridptr); 
		printf(", pageptr=0x%x, recptr=%0x%x", pageptr, recptr); 
		printf(", trans_id=%d, lockup=%c, ",trans_id, lockup ? 'T':'F');
		printf(")\n");
	}
#endif

	/* read the page in */
	GETPID(pid, *ridptr);		/* extract PID from RID */

	/* BF_event(CurrentTask->ncb_name,"getrec",&pid,-1); */

	/* lock the page containing the record in the required access mode */
	if (lockup)
	{
	    e = lock_page (trans_id, FC_FILEID(filenum), pid, mode, 
		COMMIT, cond);
	    CHECKERROR(e);
	}

	slotno = ridptr->Rslot;		/* extract slot number */
	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, &dp); 
	CHECKERROR(e);

	if (slotno < 0 || slotno >= dp->ridcnt) 
		e = e2BADSLOTNUMBER;	/* bad slot number */
	else if (dp->slot[-slotno] == EMPTYSLOT)
		e = e2BADSLOTNUMBER;	/* bad slot number */
	else /* find the location of the record on the page */
		*recptr = (RECORD *)&(dp->data[dp->slot[-slotno]]);

	if (e < eNOERROR) (void) bf_freebuf(filenum, &pid, dp);
	else *pageptr = dp; /* return a pointer to the page buffer */

	return(e);

}	/* r_getslot */


r_getrecord(filenum, ridptr, pageptr, recptr, trans_id, lockup, mode, cond)
int		filenum;	/* open file number */
RID		*ridptr;	/* *RID */
DATAPAGE	**pageptr;	/* out parameter - pointer to page buffer */
RECORD		**recptr;	/* out parameter - record address */
int             trans_id;       
short           lockup;        
LOCKTYPE	mode;
short           cond;   

/* Given a RID, locate the record and return its memory address.
   If the record has been moved, follow the forwarding address to find it.

   Returns:
	record address (via recptr)
	page address (via pageptr) 

   Side Effects:
	fix the buffer which holds the record

   Errors:
	e2BADSLOTNUMBER - invalid slot number
*/
{
	int		e;	/* for returned errors */
	PID		pid;	/* the page the record is on */
	int		slotno;	/* which slot on the page */
	SHORTPID	newaddr;/* the page the record has moved to */
	RECORD		*sptr;	/* slot pointer */
	DATAPAGE	*dp;	/* pointer to the page buffer */
	int		pageWasPinned;
	
#ifdef TRACE
	if (checkset(&Trace2, tGETRECORD)) {
		printf("r_getrecord(filenum=%d%, RID=", filenum);
		PRINTRIDPTR(ridptr); 
		printf(", pageptr=0x%x, recptr=%0x%x,\n", pageptr, recptr); 
		printf("trans_id=%d,lockup=%c,mode=",trans_id,lockup ? 'T':'F');
		printf(")\n");
	}
#endif

	/* read the page in */
	GETPID(pid, *ridptr);		/* extract PID from RID */

	/* lock the page containing the record in the required access mode */
	if (lockup)
	{
	    e = lock_page (trans_id, FC_FILEID(filenum), pid, mode, COMMIT, 
		cond);
	    CHECKERROR(e);
	}

	slotno = ridptr->Rslot;		/* extract slot number */
	/* test if the last page pinned is the desired page */
	if ((F_LASTPINNED(filenum)==TRUE) && 
		PIDEQ(F_LASTPIDPINNED(filenum), pid))
	{
	/*
		printf("desired page was pinned\n");
	*/
		/* already had the desired page pinned */
		dp = (DATAPAGE *) F_BUFADDR(filenum);
		pageWasPinned = TRUE;
	}
	else
	{
/*
	    printf("page not pinned\n");
	    printf("desired pid=%d.%d, lastpinned=%d.%d, lastpinnbool=%d\n",
	    	pid.Pvolid, pid.Ppage, F_LASTPIDPINNED(filenum).Pvolid,
	     	F_LASTPIDPINNED(filenum).Ppage, F_LASTPINNED(filenum));
*/
	    if (F_LASTPINNED(filenum)==TRUE)
	    {
	     	bf_freebuf(filenum, &F_LASTPIDPINNED(filenum),
			     F_BUFADDR(filenum));
		F_LASTPINNED(filenum) = FALSE;
	    }

	    pageWasPinned = FALSE;
	    /* get the page the record is to be added */
	    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&dp);
	    CHECKERROR(e);
	}

	if (slotno < 0 || slotno >= dp->ridcnt) 
		e = e2BADSLOTNUMBER;	/* bad slot number */
	else if (dp->slot[-slotno] == EMPTYSLOT)
		e = e2BADSLOTNUMBER;	/* bad slot number */
	if (e < eNOERROR) {
		if (!pageWasPinned) (void) bf_freebuf(filenum, &pid, dp);
		return(e);
	}

	/* find the location of the record on the page */
	sptr = (RECORD *)&(dp->data[dp->slot[-slotno]]);

	/* if the record has been moved, track it down */
	if (sptr->type == MOVED) 
	{ 
	    newaddr = ((RID *)sptr->data)->Rpage;
	    slotno = ((RID *)sptr->data)->Rslot;
	    if (!pageWasPinned) e = bf_freebuf(filenum, &pid, dp);
	    	else e=eNOERROR;
	    CHECKERROR(e);
	    pid.Ppage = newaddr;
	    /* lock page containing the record in the required access mode */
	    if (lockup)
	    {
	        e = lock_page (trans_id, FC_FILEID(filenum), pid, mode, 
			COMMIT, cond);
	        CHECKERROR(e);
	    }

	    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, &dp);
	    CHECKERROR(e);
	    sptr = (RECORD *)&(dp->data[dp->slot[-slotno]]);
	}
	/* return the record pointer and the page buffer pointer */
	*recptr = sptr;
	*pageptr = dp; 

	return(eNOERROR);

}	/* r_getrecord */

