

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
/* Module st_scanfile : routine to find the next record in a file
   returning a pointer to the actual record which is left
   fixed in the buffer pool

   IMPORTS :
	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
	bf_freebuf(filenum, pageid, pagebuf)
 
   EXPORTS :
	st_scanfile(filenum, ridptr, nextrid, recptr, reclen, 
		trans_id, lockup, mode, cond);

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

st_scanfile(filenum, ridptr, nextrid, recptr, reclen, trans_id, 
	lockup, mode, cond)
int	filenum;	/* open file number */
RID	*ridptr;	/* ID of the current record */
RID     *nextrid;	/* place to return the next RID */
char	**recptr;	/* place to return pointer to next record */
int	*reclen;	/* place to return the length the record */
int     trans_id;       
int   lockup;        
LOCKTYPE mode; 
int	cond;

/* Given a RID, return the ID of the next record.
   If no current RID is given, return the ID of the first record.

   Returns:
	next RID in the file (via nextrid)
	pointer to the actual record (via recptr)

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
	int		idx;
	RECORD          *sptr;  	/* slot pointer */


#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_nextfile(filenum=%d, currentrid=", filenum);
                PRINTRIDPTR(ridptr); printf(", nextrid=0x%x)\n", nextrid);
	}
#endif
/*
	printf("st_nextfile, transid=%d, lockup=%d, mode=%d, cond=%d\n",
		trans_id, lockup, mode, cond);
*/

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
		newpage = FALSE;
	}

	/* BF_event(CurrentTask->ncb_name,"st_nextfile",&pid, 0); */


	/* loop until a valid RID is found or end of file */
        for (; ; pid.Ppage = nextpid, slotno = -1) 
	{
/*
 *  if locking is desired then each page which is to be accessed has
 *  to be locked.  So acquire the page in the given mode. Acquire the
 *  page only when it is a new page.
 */
		if ((lockup == TRUE) && (newpage == TRUE))
		{
	    	    e = lock_page (trans_id, FC_FILEID(filenum), pid, mode, 
			COMMIT, cond);
	            CHECKERROR(e);
		}

		/* test if the last page pinned is the desired page */
		if ((F_LASTPINNED(filenum)==TRUE) && 
			(PIDEQ(F_LASTPIDPINNED(filenum), pid)))
		{
		    /* already had the desired page pinned */
		    pageptr = (DATAPAGE *) F_BUFADDR(filenum);
		}
		else
		{
	    	    /* first, if some other page is currently pinned */
	            /* unpin it before getting a new page */

	   	    if (F_LASTPINNED(filenum)==TRUE)
		    {
	     	    	   bf_freebuf(filenum, &F_LASTPIDPINNED(filenum), 
				F_BUFADDR(filenum));
			   F_LASTPINNED(filenum) = FALSE;
		    }

	    	    /* get the page the record is to be added to */
	            e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		    	(PAGE **)&pageptr);
	    	    CHECKERROR(e);

	            /* mark page as pinned and save buffer address of page */
	    	    F_LASTPINNED(filenum) = TRUE;
	    	    F_LASTPIDPINNED(filenum) = pid;
	            F_BUFADDR(filenum) =  (char *) pageptr;
	    	}

		/* find the next valid rid on this page */
		while( ++slotno < pageptr->ridcnt) 
		{ 
        		if (pageptr->slot[-slotno] != EMPTYSLOT &&
			    RECTYPE(pageptr, slotno) != NEWHOME &&
			    RECKIND(pageptr, slotno) == NORMAL    ) 
			{ 
				MAKERID(nextrid, &pid, slotno);
				sptr = (RECORD *)&(pageptr->data[pageptr->slot[-slotno]]);
				*recptr = sptr->data; 
				*reclen = sptr->length;
				return(eNOERROR);
			}
		}
		/* oops no more valid rids on this page */
		nextpid = pageptr->nextpage;
		newpage = TRUE;

		/* unfix the buffer */
	   	if (F_LASTPINNED(filenum)==TRUE)
		{
	     	     bf_freebuf(filenum, &F_LASTPIDPINNED(filenum), 
				F_BUFADDR(filenum));
		     F_LASTPINNED(filenum) = FALSE;
		}
		if (nextpid == NULLPAGE) /* end of file! */
			return(e2ENDOFFILE);
        }

}	/* st_nextfile */



/* 
   Module st_firstscan : routine to obtain the rid, recptr, and length 
	of the first record in a file
   
   IMPORTS:
	st_scanfile(filenum, ridptr, nextridptr, recptr, reclen, 
		trans_id, lockup, mode, cond);

   EXPORTS:
	st_firstscan(filenum, firstrid, recptr,reclen,
		trans_id, lockup, mode, cond)

*/


st_firstscan(filenum, firstrid, recptr, reclen, trans_id, lockup, mode, cond)
int	filenum;		/* open file number */
RID	*firstrid;		/* *RID */
RECORD	**recptr;
int	*reclen;
int     trans_id;
int     lockup;
LOCKTYPE mode;
int     cond;

/* Compute the ID of the first record of the file.

   Returns:
	ID of the first record (via firstrid)

   Side Effects:
	None

   Errors:
	None

*/

{
	int e;

	/* let st_scanfile does the dirty work */
        e = st_scanfile(filenum, (RID *) NULL, firstrid, recptr, reclen, 
		trans_id, lockup, mode, cond);
        return(e);
} /* st_firstfile */
