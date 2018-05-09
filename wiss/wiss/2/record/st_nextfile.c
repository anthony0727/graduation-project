
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
/* Module st_nextfile : routine to find the next RID in the file

   IMPORTS :
	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
	bf_freebuf(filenum, pageid, pagebuf)
 
   EXPORTS :
	st_nextfile(filenum, ridptr, nextrid, trans_id, lockup, mode, cond);

*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

#define	RECTYPE(p,s)	((RECORD *)&((p)->data[(p)->slot[-s]]))->type
#define	RECKIND(p,s)	((RECORD *)&((p)->data[(p)->slot[-s]]))->kind

/*
 *  Persumes that the page which is pointed to by rid is already locked in
 *  the given mode.  If the page is null then it locks the firstpage which
 *  qualifies in the given lock mode.
 */

st_nextfile(filenum, ridptr, nextrid, trans_id, lockup, mode, cond)
int	filenum;	/* open file number */
RID	*ridptr;	/* ID of the current record */
RID     *nextrid;	/* place to return the next RID */
int     trans_id;       
short   lockup;        
LOCKTYPE mode; 
short	cond;

/* Given a RID, return the ID of the next record.
   If no current RID is given, return the ID of the first record.

   Returns:
	next RID in the file (via nextrid)

   Side Effects:
	None

   Errors:
	e2ENDOFFILE:  no more records
	e2NULLRIDPTR: no place to return the RID

*/
{
	int		e;		/* for returned errors */
	int		slotno;		/* slot # */
	PID		pid;		/* ID of the current page */
	SHORTPID	nextpid;	/* ID of the next page */
	DATAPAGE	*pageptr;	/* pointer to a page buffer */
	short           newpage;


#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_nextfile(filenum=%d, currentrid=", filenum);
                PRINTRIDPTR(ridptr); printf(", nextrid=0x%x)\n", nextrid);
	}
#endif

	/* check input parameters */
	CHECKOFN(filenum);
	if (nextrid == NULL) return(e2NULLRIDPTR);

	if (ridptr == NULL || TESTRIDCLEAR(*ridptr)) { 
		RIDCLEAR(*nextrid);
		/* start from the begining of the file */
    		pid = F_FIRSTPID(filenum);

		if (TESTPIDCLEAR(pid))    /* an empty file ! */
			return(e2ENDOFFILE);
		slotno = -1;
		newpage = TRUE;
	}
	else { /* the following order is important when ridptr=nextrid! */
        	GETPID(pid, *ridptr); 	/* get PID from *rid */
		slotno = ridptr->Rslot;
		RIDCLEAR(*nextrid);

/*
 *  SHERROR possible source of error where if the page which rid is 
 *  pointing to is not locked, over here I am assuming it is locked.
 */

		newpage = FALSE;
	}

	/* BF_event(CurrentTask->ncb_name,"st_nextfile",&pid, 0); */


	/* loop until a valid RID is found or end of file */
        for (; ; pid.Ppage = nextpid, slotno = -1) {

/*
 *  if locking is desired then each page which is to be accessed has
 *  to be locked.  So acquire the page in the given mode. Acquire the
 *  page only when it is a new page.
 */
		if ((lockup == TRUE) && (newpage == TRUE))
		{
	    	    e = lock_page (trans_id, FC_FILEID(filenum), pid, mode, 
			COMMIT, cond, TRUE);
	            CHECKERROR(e);
		}

		/* get the next page in */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, &pageptr); 
		CHECKERROR(e);

		/* find the next valid rid on this page */
		while( ++slotno < pageptr->ridcnt) { 
        		if (pageptr->slot[-slotno] != EMPTYSLOT &&
			    RECTYPE(pageptr, slotno) != NEWHOME &&
			    RECKIND(pageptr, slotno) == NORMAL    ) { 
				MAKERID(nextrid, &pid, slotno);
				bf_freebuf(filenum, &pid, pageptr);
				return(eNOERROR);
			}
		}
		nextpid = pageptr->nextpage;
		newpage = TRUE;

		/* unfix the buffer */
		(void) bf_freebuf(filenum, &pid, pageptr);
		if (nextpid == NULLPAGE) /* end of file! */
			return(e2ENDOFFILE);
        }

}	/* st_nextfile */

