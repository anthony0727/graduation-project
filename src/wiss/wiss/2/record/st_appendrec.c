
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
/* Module st_appendrecord : append a record to a file

   IMPORTS:
	r_addrec(filenum, onpid, recaddr, reclen, newridptr, append, trans_id,
		 lockup)
	r_hookup(filenum, firstpid, secondpid, newpid, trans_id, lockup)

   Exports:
	st_appendrecord(filenum, recaddr, reclen, newrid, trans_id, 
		lockup, cond)
*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


st_appendrecord(filenum, recaddr, reclen, newrid, trans_id, lockup, cond)
int	filenum;	/* index into the open file table */
char	*recaddr;	/* text of the new record */
int	reclen;		/* length of the new record */
RID	*newrid;	/* where to return the new born RID */
int     trans_id;    
short   lockup;     
short   cond;         

/* Append a record to a file.

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

#ifdef	TRACE
	if (checkset(&Trace2,tINTERFACE))
		printf("st_appendrec(filenum=%d,recaddr=0x%x,reclen=%d,trans_id %d, lockup %d) ",
			filenum, recaddr, reclen,trans_id,lockup, cond);
#endif

	/* check input parameters */
	CHECKOFN(filenum);	/* check if valid */
	CHECKWP(filenum);	/* check write permission */
	if (recaddr == NULL) return(e2NULLRECADDR);
	if (reclen <= 0) return(0);

    	pid = F_LASTPID(filenum);

	/* append to the last page if there is room */
	if(!TESTPIDCLEAR(pid))	{ 
	    e = r_addrec(filenum, &pid, recaddr, reclen, newrid, TRUE, 
		trans_id, lockup, cond);
	    done = (e >= eNOERROR);
	}

	if (!done) /* need a new page at the end of the file */
	{ 
	    if (lockup)
	    {
		/* must upgrade lock on file to exclusive when adding a new 
		page to the end of the file  */
		/* DeWItt  - why ??? - why not just lock the last page */
	        e = lock_file (trans_id, FC_FILEID(filenum), l_X, COMMIT, cond);
	        CHECKERROR(e);
	    }
    	    pid = F_LASTPID(filenum);

/* since the file has been locked in exclusive mode in this case, there
is no reason for r_hookup() to lock the page */
/* DeWitt.  while this is true, the code keeps calling append_rec
with lockup equal to true and thus page locks are set on every
single page but the first page of each extent in the file.  I 
changed lockup in the following call from FALSE to lockup
as to really fix this a major change is needed to get things
correct */

	    e = r_hookup(filenum, &pid, (PID *) NULL, &newpid, 
		trans_id, lockup, cond);
	    CHECKERROR(e);

/*
 *  since the newly created page has already been locked by r_hookup it does not 
 *  make sense for r_addrec to lock the page to lock it again so pass a FALSE flag
 *  in.
 */
	    e = r_addrec(filenum, &newpid, recaddr, reclen, newrid, TRUE, 
		trans_id, FALSE, cond);
	    CHECKERROR(e);
	}
	/* update file stats */
	if (F_FILETYPE(filenum) == DATAFILE)
	{
		F_CARD(filenum)++, F_STATUS(filenum) = DIRTY;
	}

	return(eNOERROR);

} /* st_appendrecord */

