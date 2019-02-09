
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
/* Module addrec :
	This routine adds a record to a given page. If the record won't fit, 
	it simply gives up and returns an error code to the caller.
	It is then the caller's responsibility to determine what to do next.
	If "append" is TRUE, then put the record at the very end of the page.

   IMPORTS:
        bf_readbuf(trans_id, filenum, fid, pageid, returnpage);
        bf_freebuf(filenum, pageid, pagebuf);
	bf_setdirty(filenum, pageid, pagebuf)
	r_slide(pageptr, slot, reclen, recptr, trans_id, lockup)

   EXPORTS:
	r_addrec(filenum, onpid, recstart, reclen, newrid, append, 
		trans_id, lockup, cond)
*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

r_addrec(filenum, onpid, recstart, reclen, newridptr, append, trans_id, lockup, cond)
int	filenum;		/* which file */
PID	*onpid;			/* which page to put the record */
char	*recstart;		/* start of record text */
int	reclen;			/* length of record */
RID	*newridptr;		/* out parameter, RID of the new record */
int	append, trans_id;	/* TRUE if the record should put at the end */
short   lockup;                 /* if TRUE the routine does the locking */
short   cond;                   /* conditional locking flag */

/* Add a record to a page, if it won't fit, return an error code.

   Returns:
	the RID of the newly added record (via newridptr)

   Side Effects:
	a new record is born if there is room for it

   Errors:
	e2RECWONTFIT:  not enough room on this page
	e2PAGENOTINFILE: the page referenced is not in the file

*/
{
	register	e;		/* for returned errors */
	register	slot;		/* the new slot for the new record */
	DATAPAGE	*dp;		/* where the record is to be added */
	RECORD		*putptr;	/* where we put the record */
	float		freefraction;	/* fraction of a page to leave free */
	int		pff;
	PID		pid;
	int		pageWasPinned;


#ifdef TRACE
	if (checkset(&Trace2,tADDREC)) {
		printf("r_addrec(filenum=%d,",filenum);
		printf("onpid="); PRINTPIDPTR(onpid);
		printf("recstart=0x%x,reclen=%d,*newrid=0x%x,append=%c, trans_id,lockup=%c)\n", 
		 recstart, reclen, newridptr, 
		 append ? 'T':'F',trans_id, lockup ? 'T':'F');
	}
#endif

	/* BF_event (CurrentTask->ncb_name, "r_addrec", reclen, trans_id); */
	/* get the page the record is to be added */

	/* lock the page if required, since update, lock page in X mode */
	if (lockup)
	{
	    e = lock_page (trans_id, FC_FILEID(filenum), *onpid, l_X, COMMIT, 
		cond);
	    CHECKERROR(e);	
	}
	pid = *onpid;

	/* test if the last page pinned is the desired page */
	if ((F_LASTPINNED(filenum)==TRUE) && 
		PIDEQ(F_LASTPIDPINNED(filenum), pid))
	{
		/* already had the desired page pinned */
		dp = (DATAPAGE *) F_BUFADDR(filenum);
		pageWasPinned = TRUE;
	}
	else
	{
	    /* wrong page is fixed - need to get the right one */
	    /* get the page the record is to be added */
	    pageWasPinned = FALSE;

	    if (F_LASTPINNED(filenum)==TRUE)
	    {
		 bf_freebuf(filenum, &F_LASTPIDPINNED(filenum),
	         F_BUFADDR(filenum));
		 F_LASTPINNED(filenum) = FALSE;
	    }
	    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), onpid,
		(PAGE **)&dp);
	    CHECKERROR(e);
	}

	/* check if the page belongs to the file */
	if (!FIDEQ(F_FILEID(filenum), dp->fileid)) 
	{
	        F_LASTPINNED(filenum) = FALSE;
		(void) bf_freebuf(filenum, onpid, dp); /* unfix the buffer */
		return(e2PAGENOTINFILE);
	}

	/* in the case of append operations, we use the page fill 
	* factor associated with the file control how full the pages
	* should be */

	if (append) 
	{
	    pff = F_PFF(filenum);
	    freefraction = 1.0 - ((float)pff)/100.0;
	    if (DATAFREE(dp) < (int) (PAGESIZE*freefraction))
	    {
	        /* was F_LASTPINNED(filenum) = FALSE; */
		/* unfix the buffer */
		if (!pageWasPinned) (void) bf_freebuf(filenum, onpid, dp); 
		return(e2RECWONTFIT);	/* not enough room */
	    }
	}

	/* check if there is enough room on the page */
	if ((DATAFREE(dp)) < ((int) sizeof(dp->slot[0]) + HEADERLEN + reclen)) 
	{
	        /* F_LASTPINNED(filenum) = FALSE; */
		/* unfix the buffer */
		if (!pageWasPinned) (void) bf_freebuf(filenum, onpid, dp); 
		return(e2RECWONTFIT);	/* not enough room */
	}

	/* decide which slot to use */
	if (append) slot = dp->ridcnt;	/* use the last slot */
	else for (slot = 0; slot < dp->ridcnt; slot++) /* find an empty slot */
		if (dp->slot[-slot] == EMPTYSLOT) break;
	if (slot == dp->ridcnt) /* the new slot is the last slot on page */
		dp->slot[-(dp->ridcnt++)] = EMPTYSLOT;	

	/* make room for the record */
	/* Since page is already locked in X mode r_slide need no locking. */
	e = r_slide(dp, slot, reclen, &putptr, trans_id, FALSE, cond);

	if (e < eNOERROR) { /* free the slot and return an error code */
		printf ("r_slide did mess up.\n");
	        /* F_LASTPINNED(filenum) = FALSE; */
		/* unfix the buffer */
		if (!pageWasPinned) (void) bf_freebuf(filenum, onpid, dp); 
		return(e);
	}

	/* copy data area and set record type (length is set by r_slide) */
	movebytes(putptr->data, recstart, reclen);	
	putptr->type = NOTMOVED, putptr->kind = NORMAL;

	/* return the new RID */
	if (newridptr != NULL) {
		MAKERID(newridptr, onpid, slot);
	}

#ifdef DEBUG
	if (checkset(&Trace2,tDATAPAGE))
		r_dumppage(dp, trans_id, lockup);	/* dump this page */
#endif

	(void) bf_setdirty(filenum, onpid, dp);	/* inform level 1 */
	if (!pageWasPinned)  e = bf_freebuf(filenum, onpid, dp); 
	return(e);

} /* r_addrec */

