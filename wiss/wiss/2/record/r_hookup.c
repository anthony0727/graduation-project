
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
/* Module r_hookup : Adds a new page to a file between two given pages.

   IMPORTS:
	io_allocapges(fid, pid, numpages, newpid)
	bf_getbuf(trans_id, filenum, fid, pageid, returnpage)
        bf_readbuf(trans_id, filenum, fid, pageid, returnpage);
        bf_freebuf(filenum, pageid, pagebuf);
	bf_setdirty(filenum, pageid, pagebuf)

   EXPORTS:
	r_hookup(filenum, firstpid, secondpid, newpid, trans_id, lockup, cond)
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>


r_hookup (filenum, firstpid, secondpid, newpid, trans_id, lockup, cond)
int	filenum;		/* which file */
PID	*firstpid,*secondpid;	/* pages to be hooked-up */
PID	*newpid;		/* pid of the new page. */
int     trans_id;              
short   lockup;                 
short   cond;

/* Allocate a new page and add it to the file between two given pages.

   Returns:
	None

   Side Effects:
	the assoicated file descriptor may be updated in cases where
	the page added is the new first page and/or the new last page

   Errors:
	None
*/
{
	int 	e; 		/* for returned errors. */
	FID 	fid; 		/* the internal file ID */
	short 	nullprior; 	/* no prior page */
	short	nullnext; 	/* no next page */
	PID 	*pidptr; 	
	DATAPAGE *pageptr; 

#ifdef TRACE
	if ( checkset(&Trace2,tPAGELINKS)) {
		printf("r_hookup(filenum = %d,",filenum);
		printf("firstpid="); PRINTPIDPTR(firstpid);
		printf(",secondpid="); PRINTPIDPTR(secondpid);
		printf(",newpid=0x%x)\n",newpid);
	}
#endif

	/* get the internal file ID */
	fid = F_FILEID(filenum);	

	nullprior = (firstpid == NULL) ? TRUE : TESTPIDCLEAR(*firstpid);
	nullnext = (secondpid == NULL) ? TRUE : TESTPIDCLEAR(*secondpid);


	/* find a PID as reference for allocating a nearby page */
	if (!nullprior) pidptr = firstpid;
	else if (!nullnext) pidptr = secondpid;	 
	else pidptr = NULL; 		/* neither PIDs are real */

	/* allocate a new page */

	e = io_allocpages(&fid, pidptr, 1, newpid);
	CHECKERROR(e);

/*
 *  Notice locking should be done right after io_allocpages, since at this
 *  point of time newpid has no real identity and only after 
 *  io_allocpages it gains identity.
 *  the locking should be done in X mode for newpid no matter what
 */
	if (lockup)
	{
	    e = lock_page (trans_id, fid, *newpid, l_X, COMMIT, cond);
	    CHECKERROR(e);
	}

	/* lock the related pages before fixing the pointers */
	if (!nullprior && lockup == TRUE)
	{
	    e = lock_page (trans_id, fid, *firstpid, l_X, COMMIT, cond);
	    CHECKERROR(e);
	}
	if (!nullnext && lockup == TRUE)
	{
	    e = lock_page (trans_id, fid, *secondpid, l_X, COMMIT, cond);
	    CHECKERROR(e);
	}

	if (!nullprior) { /* let the previous page point to the new page */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum),
			firstpid, &pageptr);	
		CHECKERROR(e);
		pageptr->nextpage = newpid->Ppage;
		(void) bf_setdirty(filenum, firstpid, pageptr);
		e = bf_freebuf(filenum, firstpid, pageptr);
		CHECKERROR(e);
	}
	else /* set new first PID */
	{
		F_FIRSTPID(filenum) = *newpid;
		F_STATUS(filenum) = DIRTY;
	}

	if (!nullnext) { /* let the next page point to the new page */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum),
			secondpid, &pageptr);	
		CHECKERROR(e);
		pageptr->prevpage = newpid->Ppage;
		(void) bf_setdirty(filenum, secondpid, pageptr);
		e = bf_freebuf(filenum, secondpid, pageptr);	
		CHECKERROR(e);
	}
	else /* set new last PID */
	{
		F_LASTPID(filenum) = *newpid; 
		F_STATUS(filenum) = DIRTY;
	}

	/* initialize the new page */
	e = bf_getbuf(trans_id, filenum, FC_FILEID(filenum),
		newpid, &pageptr);
	CHECKERROR(e);
	pageptr->fileid = fid;
	pageptr->thispage = *newpid;
	pageptr->ridcnt = pageptr->free = 0; 
	pageptr->prevpage = nullprior ? NULLPAGE : firstpid->Ppage;
	pageptr->nextpage = nullnext ? NULLPAGE : secondpid->Ppage;
	e = bf_freebuf(filenum, newpid, pageptr);
	CHECKERROR(e);

	if (F_FILETYPE(filenum) == DATAFILE) /* update file descriptor */
		F_NUMPAGES(filenum)++, F_STATUS(filenum) = DIRTY;

	return(eNOERROR);

} /* r_hookup */

