
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
/* Module st_appendfile : append a record to a file

   This is a special version of st_appendrec that keeps the last 
   of the file pinned to improved performance 

   IMPORTS:
	r_addrec(filenum, onpid, recaddr, reclen, newridptr, append, trans_id,
		 lockup)
	r_hookup(filenum, firstpid, secondpid, newpid, trans_id, lockup)

   Exports:
	st_appendfile(filenum, recaddr, reclen, newrid, trans_id, 
		lockup, cond)
*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


st_appendfile(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)
int	filenum;	/* index into the open file table */
char	*recaddr;	/* text of the new record */
int	reclen;		/* length of the new record */
RID	*newrid;	/* where to return the new born RID */
int     trans_id;    
short   lockup;     
short   cond;         

/* Append a record to a file. - keeping the last page pinned between calls 

   Returns:
	RID of the newly created record

   Side effects:
	None

   Errors:
	e2NULLRECADDR - null record address
*/
{
	int	e;	/* for returned errors */
	int	done = FALSE;
	PID	pid, newpid;
	DATAPAGE    *dp; /* pointer to a page buffer */
	int	idx;


#ifdef	TRACE
	if (checkset(&Trace2,tINTERFACE))
		printf("st_appendrec(filenum=%d,recaddr=0x%x,reclen=%d,trans_id %d, lockup %d) ",
			filenum, recaddr, reclen,trans_id,lockup, cond);
#endif

	/* check input parameters */
	CHECKOFN(filenum);	/* check if valid */
	CHECKWP(filenum);	/* check write permission */
	if (recaddr == NULL) return(e2NULLRECADDR);
	if (reclen < 0) return(0);

    	pid = F_LASTPID(filenum);

	/* append to the last page if there is room */
	if(!TESTPIDCLEAR(pid))	{ 
	    e = r_addrec(filenum, &pid, recaddr, reclen, newrid, TRUE, 
		trans_id, lockup, cond);
	    done = (e >= eNOERROR);
	}
		
	if (!done) /* need a new page at the end of the file */
	{ 
	    /* first unpin the current page at the end of the file */
	    if ((F_LASTPINNED(filenum)==TRUE) && 
		(PIDEQ(F_LASTPIDPINNED(filenum), pid)))
	    {
	        e = bf_freebuf(filenum, &F_LASTPIDPINNED(filenum), 
			F_BUFADDR(filenum));
	        CHECKERROR(e);
		F_LASTPINNED(filenum) = FALSE;
	    }

	    if (lockup)
	    {
		/* must upgrade lock on file to exclusive when adding a new 
		page to the end of the file  */
		/* DeWitt  - why ??? - why not just lock the last page */
	        e = lock_file (trans_id, FC_FILEID(filenum), l_X, COMMIT, cond);
	        CHECKERROR(e);
	    }
    	    pid = F_LASTPID(filenum);

	    e = r_hookup(filenum, &pid, (PID *) NULL, &newpid, 
		trans_id, lockup, cond);
	    CHECKERROR(e);

	    /*
 	    * since the newly created page has already been locked by r_hookup 
 	    * it does not make sense for r_addrec to lock the page to lock 
 	    * it again so pass a FALSE lockup flag
 	    */
	    e = r_addrec(filenum, &newpid, recaddr, reclen, newrid, TRUE, 
		trans_id, FALSE, cond);
	    CHECKERROR(e);
	    pid = newpid;
	}
	/* update file stats */
	if (F_FILETYPE(filenum) == DATAFILE)
	{
		F_CARD(filenum)++, F_STATUS(filenum) = DIRTY;
	}

	if ((F_LASTPINNED(filenum)==TRUE) && 
		(!PIDEQ(F_LASTPIDPINNED(filenum), pid)))
	{
	    /* got the wrong page pinned  - unpin it */
	    idx = ((PAGE *) F_BUFADDR(filenum)) -  smPtr->bufferpool;
	    if (PIDEQ(F_LASTPIDPINNED(filenum), smPtr->buftable[idx].Bpageid))
	    {
	     	  e= bf_freebuf(filenum, &F_LASTPIDPINNED(filenum), 
				F_BUFADDR(filenum));
		  CHECKERROR(e);
	    }
	    F_LASTPINNED(filenum) = FALSE;
	}
	if (F_LASTPINNED(filenum) == FALSE)
	{
	    /* pin the last page so that when we add the next record */
	    /* r_addrec will find it pinned */
	    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, &dp);
	    CHECKERROR(e);

	    /* mark page as pinned and save buffer address of page */
	    F_LASTPINNED(filenum) = TRUE;
	    F_LASTPIDPINNED(filenum) = pid;
	    F_BUFADDR(filenum) = (char *) dp;
	}
	return(eNOERROR);
} /* st_appendfile */

st_appendnewfile(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)
int	filenum;	/* index into the open file table */
char	*recaddr;	/* text of the new record */
int	reclen;		/* length of the new record */
RID	*newrid;	/* where to return the new born RID */
int     trans_id;
short   lockup;
short   cond;

/* Append a record to a file. - keeping the last page pinned between calls

Returns:
RID of the newly created record

Side effects:
None

Errors:
e2NULLRECADDR - null record address
*/
{
	int	e;	/* for returned errors */
	int	done = FALSE;
	PID	pid, newpid;
	DATAPAGE    *dp; /* pointer to a page buffer */
	int	idx;


#ifdef	TRACE
	if (checkset(&Trace2, tINTERFACE))
		printf("st_appendrec(filenum=%d,recaddr=0x%x,reclen=%d,trans_id %d, lockup %d) ",
		filenum, recaddr, reclen, trans_id, lockup, cond);
#endif

	/* check input parameters */
	CHECKOFN(filenum);	/* check if valid */
	CHECKWP(filenum);	/* check write permission */
	if (recaddr == NULL) return(e2NULLRECADDR);
	if (reclen < 0) return(0);

	pid = F_LASTPID(filenum);

		/* first unpin the current page at the end of the file */
		if ((F_LASTPINNED(filenum) == TRUE) &&
			(PIDEQ(F_LASTPIDPINNED(filenum), pid)))
		{
			e = bf_freebuf(filenum, &F_LASTPIDPINNED(filenum),
				F_BUFADDR(filenum));
			CHECKERROR(e);
			F_LASTPINNED(filenum) = FALSE;
		}

		if (lockup)
		{
			/* must upgrade lock on file to exclusive when adding a new
			page to the end of the file  */
			/* DeWitt  - why ??? - why not just lock the last page */
			e = lock_file(trans_id, FC_FILEID(filenum), l_X, COMMIT, cond);
			CHECKERROR(e);
		}
		pid = F_LASTPID(filenum);

		e = r_hookup(filenum, &pid, (PID *)NULL, &newpid,
			trans_id, lockup, cond);
		CHECKERROR(e);

		/*
		* since the newly created page has already been locked by r_hookup
		* it does not make sense for r_addrec to lock the page to lock
		* it again so pass a FALSE lockup flag
		*/
		e = r_addrec(filenum, &newpid, recaddr, reclen, newrid, TRUE,
			trans_id, FALSE, cond);
		CHECKERROR(e);
		pid = newpid;

	/* update file stats */
	if (F_FILETYPE(filenum) == DATAFILE)
	{
		F_CARD(filenum)++, F_STATUS(filenum) = DIRTY;
	}

	if ((F_LASTPINNED(filenum) == TRUE) &&
		(!PIDEQ(F_LASTPIDPINNED(filenum), pid)))
	{
		/* got the wrong page pinned  - unpin it */
		idx = ((PAGE *)F_BUFADDR(filenum)) - smPtr->bufferpool;
		if (PIDEQ(F_LASTPIDPINNED(filenum), smPtr->buftable[idx].Bpageid))
		{
			e = bf_freebuf(filenum, &F_LASTPIDPINNED(filenum),
				F_BUFADDR(filenum));
			CHECKERROR(e);
		}
		F_LASTPINNED(filenum) = FALSE;
	}
	if (F_LASTPINNED(filenum) == FALSE)
	{
		/* pin the last page so that when we add the next record */
		/* r_addrec will find it pinned */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, &dp);
		CHECKERROR(e);

		/* mark page as pinned and save buffer address of page */
		F_LASTPINNED(filenum) = TRUE;
		F_LASTPIDPINNED(filenum) = pid;
		F_BUFADDR(filenum) = (char *)dp;
	}
	return(eNOERROR);
} /* st_appendnewfile */
