
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
/* Module r_dropout : unlink and remove a page from a file 

   IMPORTS:
	io_freepage(fid, pid)
        bf_readbuf(trans_id, filenum, fid, pageid, returnpage);
	bf_setdirty(filenum, pageid, pagebuf)
	bf_freebuf(filenum, pageid, pagebuf)
	bf_discard(filenum, pageid, pagebuf)
	lock_page (trans_id, page_id, mode, commit, cond)

   Exports:
	r_dropout(filenum, pidptr, trans_id, lockup, cond)

*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>

r_dropout(filenum, pidptr, trans_id, lockup, cond)
int	filenum;	/* which file */
PID	*pidptr;	/* page to be deleted */
int     trans_id;       /* the transaction requesting action */
short   lockup;         /* if true the routine takes appropriate actions for locking */
short    cond;

/* Unlink and remove a page from a file.

   Returns:
	None

   Side Effects:
	the file descriptor may be updated

   Errors:
	None

*/
{
	int	e;	/* for returned errors */
	FID	fid;	/* the internal file ID */
	PID	prevpid;/* pid of the previous page */
	PID	nextpid;/* pid of the next page */
	DATAPAGE *dp;

#ifdef TRACE
	if ( checkset(&Trace2,tPAGELINKS) ) {
		printf("r_dropout(filenum = %d,",filenum);
		printf("pid = "); PRINTPIDPTR(pidptr); printf(")\n");
		printf("trans_id = %d, lockup = %c",trans_id,lockup ? 'T':'F');
	}
#endif

	/* get the internal file ID */
	fid = FC_FILEID(filenum); 

	/* lock the page if lockup is true */
	if (lockup) {
	    e = lock_page(trans_id, fid, *pidptr, l_X, COMMIT, cond);
	    CHECKERROR(e);
	}

	/* get the page, find its neighbors and remove the page */
	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), pidptr, &dp);	
	CHECKERROR(e);
	prevpid.Ppage = dp->prevpage;
	prevpid.Pvolid = (prevpid.Ppage == NULLPAGE)? -1 : fid.Fvolid;
	nextpid.Ppage = dp->nextpage;
	nextpid.Pvolid = (nextpid.Ppage == NULLPAGE)? -1 : fid.Fvolid;

	e = bf_discard(filenum, pidptr, dp);
	CHECKERROR(e);
	e = io_freepage(&fid, pidptr);	
	CHECKERROR(e);

	/* Since the page is being unlinked lock the adjacent pages too */
	if (lockup) {
	  if (prevpid.Ppage != NULLPAGE)
	  {
	        e = lock_page(trans_id, fid, prevpid, l_X, COMMIT, cond);
	        CHECKERROR(e);
	  }
	
	  if (nextpid.Ppage != NULLPAGE)
	  {
	        e = lock_page(trans_id, fid, nextpid, l_X, COMMIT, cond);
	        CHECKERROR(e);
	  }
	}

	/* adjust links in the neighboring pages */
	if (prevpid.Ppage == NULLPAGE) {
	   /* we are removing the first page, the next page is now the first */
	   /* Raise the lock mode of the file to eXclusive mode */
	   /* DeWitt:     why????????  - this is correct but too strong */
	   /* should be fixed !!!!!  */
	   if (lockup) {
		e = lock_file(trans_id, fid, l_X, COMMIT, cond);
		CHECKERROR(e);
	    }
	    F_FIRSTPID(filenum) = nextpid; 
	    F_STATUS(filenum) = DIRTY;
	}
	else { /* read in the previous page, and update its nextpage pointer */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), 
			&prevpid, &dp);
		CHECKERROR(e);
		dp->nextpage = nextpid.Ppage; 	/* update link */
		(void) bf_setdirty(filenum, &prevpid, dp);/* inform level 1 */
		e = bf_freebuf(filenum, &prevpid, dp);
		CHECKERROR(e);
	}
	if (nextpid.Ppage == NULLPAGE) {
	   /* this was the last page, the previous page is now the last */
	   /* this is safe because we have a lock on the both last page */
	   /* and the previous page */
		F_LASTPID(filenum) = prevpid; 
		F_STATUS(filenum) = DIRTY;
	}
	else { /* read in the next page, and update its prevpage pointer */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), 
			&nextpid, &dp);
		CHECKERROR(e);
		dp->prevpage = prevpid.Ppage; 	/* update link */
		(void) bf_setdirty(filenum, &nextpid, dp);/* inform level 1 */
		e = bf_freebuf(filenum, &nextpid, dp);
		CHECKERROR(e);
	}

	if (F_FILETYPE(filenum) == DATAFILE) /* update file descriptor */
		F_NUMPAGES(filenum)--, F_STATUS(filenum) = DIRTY;

	return(eNOERROR);

} /* r_dropout */

