
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
/* Module st_prevfile : routine to return the previous RID in the file

   IMPORTS :
	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
	bf_freebuf(filenum, pageid, pagebuf)
 
   EXPORTS :
	st_prevfile(filenum, ridptr, prevrid, trans_id, lockup, mode);

*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

#define	RECTYPE(p,s)	((RECORD *)&((p)->data[(p)->slot[-s]]))->type
#define	RECKIND(p,s)	((RECORD *)&((p)->data[(p)->slot[-s]]))->kind

st_prevfile(filenum, ridptr, prevrid, trans_id, lockup, mode, cond)
int	filenum;	/* open file number */
RID	*ridptr;	
RID     *prevrid;
int     trans_id;       
short   lockup;        
LOCKTYPE mode;
short   cond;        

/*
   Returns:
	previous RID in the file 

   Side Effects:
	None

   Errors:
	e2ENDOFFILE: no more records
	e2NULLRIDPTR

*/
{
	int		e;
	register int	slotno;
	PID		pid;
	SHORTPID	prevpid;
	DATAPAGE	*pageptr;
	short           newpage;

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_prevfile(filenum=%d, currentrid =", filenum);
                PRINTRIDPTR(ridptr); printf(", prevrid=0x%x)\n", prevrid);
	}
#endif

	CHECKOFN(filenum);
	if (prevrid == NULL) return(e2NULLRIDPTR);

	if (ridptr == NULL || TESTRIDCLEAR(*ridptr) ) {
		RIDCLEAR(*prevrid);

    		pid = F_LASTPID(filenum);

		if (TESTPIDCLEAR(pid))    /* an empty file ! */
			return(e2ENDOFFILE);
		slotno = -1;
		newpage = TRUE;
	}
	else { /* the following order is important when ridptr = prevrid */
        	GETPID(pid,*ridptr); /* get PID from *rid */
		slotno = ridptr->Rslot;
		RIDCLEAR(*prevrid);
		newpage = FALSE;
	}

/*
 *  If locking is specified, might as well lock the first one right
 *  here before proceeding any longer.
 */

	if ((lockup == TRUE) && (newpage == TRUE) )
	{
	    e = lock_page (trans_id, FC_FILEID(filenum), pid, mode, COMMIT, 
		cond);
	    CHECKERROR(e);
	}

	/* get the first possible page */
	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, &pageptr);
	CHECKERROR(e);
	if (slotno < 0) slotno = pageptr->ridcnt;

	/* loop until a valid RID found or end/bottom of file */
        for (;; slotno = pageptr->ridcnt) {
		/* find the previous valid rid on this page */
		while( --slotno >= 0) { 
        		if (pageptr->slot[-slotno] != EMPTYSLOT &&
			    RECTYPE(pageptr, slotno) != NEWHOME && 
			    RECKIND(pageptr, slotno) == NORMAL) { 
				MAKERID(prevrid, &pid, slotno);
				e = bf_freebuf(filenum, &pid, pageptr);
				return(e);
			}
		}
		prevpid = pageptr->prevpage;

		newpage = TRUE;

		/* unfix the buffer */
		e = bf_freebuf(filenum, &pid, pageptr);
		CHECKERROR(e);

		if (prevpid == NULLPAGE)  /* end of file! */
			return(e2ENDOFFILE);

		/* try the previous page */
		pid.Ppage = prevpid;

		if (lockup)
		{
	    	    e = lock_page (trans_id, FC_FILEID(filenum), pid, mode, 
			COMMIT, cond);
	    	    CHECKERROR(e);
		}

		e = bf_readbuf(trans_id, filenum,FC_FILEID(filenum),&pid, 
			&pageptr); /* next page to search */
		CHECKERROR(e);
        }

}	/* st_prevfile */

