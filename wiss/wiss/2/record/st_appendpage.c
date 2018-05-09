
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
/* Module st_appendpage : append a page to a file
   IMPORTS:
        bf_readbuf(trans_id, filenum, fid, pageid, returnpage);
        bf_freebuf(filenum, pageid, pagebuf);
	bf_setdirty(filenum, pageid, pagebuf)
	r_hookup(filenum, priorpid, nextpid, newpid, transid, lockup, cond);

   Exports:
	st_appendpage(filenum, pageptr, transid, lockup, cond)

   Side Effects:
	the assoicated file descriptor may be updated in cases where
*/

#include	<wiss.h>
#include	<st.h>


st_appendpage (filenum, apage, trans_id, lockup, cond)
int	filenum;		/* which file */
DATAPAGE *apage;		/* pointer to page to be appended. */
int	trans_id;
short	lockup;
short	cond;
{
	int 	e; 		/* for returned errors. */
	PID	newpid;		/* new PID allocated and appended */
	PID	lastpage;	/* last pid of the file */
	DATAPAGE *pageptr;	/* ptr to buffer page */ 

#ifdef TRACE
	if ( checkset(&Trace2,tPAGELINKS)) {
		printf("st_appendpage(filenum = %d,",filenum);
		printf(",appendpage=0x%x)\n",apage);
	}
#endif

	/* first get the full level 2 latch */
	SetLatch(&smPtr->level2Latch, procNum, NULL);

	/* check input parameters */
	CHECKOFN(filenum);	/* check if valid */
	CHECKWP(filenum);	/* check write permission */

	if (apage == NULL) 
	{
		ReleaseLatch(&smPtr->level2Latch, procNum);
		return(e2NULLPIDPTR);
	}
	/* next get the individual file latch and release the level2Latch */
	SetLatch(F_LATCHPTR(filenum), procNum, &smPtr->level2Latch);

        lastpage = F_LASTPID(filenum);

	/* allocate/link a place for the new page */
	if (TESTPIDCLEAR(lastpage)) {
		e = r_hookup(filenum,NULL,NULL,&newpid, trans_id, 
			lockup, cond);
	}
	else {
		e = r_hookup(filenum,&lastpage,NULL,&newpid, trans_id, 
			lockup, cond);
	}
	CHECKERROR_RLATCH(e,F_LATCHPTR(filenum), procNum);

	/* copy the page into the buffer */
	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), 
		&newpid, &pageptr);	
	CHECKERROR_RLATCH(e,F_LATCHPTR(filenum), procNum);

	movebytes(pageptr->data,apage->data,PAGESIZE-DPFIXED);
	pageptr->slot[0] = apage->slot[0];
	pageptr->free	= apage->free;
	pageptr->ridcnt = apage->ridcnt;

	/* update file stats; leave page in buffer */
	F_NUMPAGES(filenum)++;
	F_CARD(filenum) += pageptr->ridcnt;
	F_STATUS(filenum) = DIRTY;
	ReleaseLatch(F_LATCHPTR(filenum), procNum);

	(void) bf_setdirty(filenum, &newpid, pageptr);
	e = bf_freebuf(filenum, &newpid, pageptr);
	CHECKERROR(e);

	return(eNOERROR);
} /* st_appendpage */

