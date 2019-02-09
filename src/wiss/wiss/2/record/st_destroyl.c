
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
/* Module st_destroylong : routine to destroy a long data item.

   IMPORTS :
	io_freepage(fid, pid)
	bf_freebuf(filenum, pageid, pageptr)
	bf_discard(filenum, pageid, pageptr)
	r_getrecord(filenum, ridptr, returnpage, recptr, trans_id, lockup, mode, 
		cond)
	st_deleterecord(filenum, rid, trans_id, lockup, cond)

   EXPORTS :
	st_destroylong(filenum, ridptr, trans_id, lockup, cond) 
*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

st_destroylong(filenum, ridptr, trans_id, lockup, cond)
int	filenum;	/* open file number */
RID	*ridptr;	/* RID of the directory */
int     trans_id;
short   lockup;
short   cond;

/* 
   This procedure destroys the long data item and releases all pages used.

   Returns:
	None

   Side Effects:
	Pages used by this long data item are released.

   Errors:
	e2NULLRIDPTR:  pointer to the directory RID is null
*/
{
	register int	i;	/* slice index */
	register int	e;	/* for returned errors */
	FID		fid;	/* level 0 file id */
	PID		pid;	/* page ID */
	LONGDIR		*dp;	/* pointer to directory */
	RECORD		*recptr;/* record pointer */
	DATAPAGE	*dirpage;/* the buffer the directory is in */
	DATAPAGE	*spage;	/* page buffer pointer for slices */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_destroylong(fileno=%d,RID=", filenum);
		PRINTRIDPTR(ridptr); printf("\n");
	}
#endif

	/* check the file number and file permission */
	CHECKOFN(filenum);
	CHECKWP(filenum);

	if (ridptr == NULL) return(e2NULLRIDPTR);

	/* read in the directory of the long data item */
	e = r_getrecord(filenum, ridptr, &dirpage, &recptr, trans_id, lockup, 
		l_X, cond);
	CHECKERROR(e);
	dp = (LONGDIR *) recptr->data;

  	/* free the slices */
	fid = F_FILEID(filenum);
	for(i = 0; i < dp->slice_count; i++) 
	{ 
	    /* read the slice in */
	    e = r_getrecord(filenum, &(dp->sptr[i].rid), &spage, &recptr, 
			trans_id, lockup, l_X, cond);
	    if (e < eNOERROR) break;

	    /* release either a slice or a crumb */
	    pid = spage->thispage;  
	    if (recptr->kind == CRUMB) 
	    {
		/* unfix the buffer then remove the record */
		(void) bf_freebuf(filenum, &pid, spage);
		/* lockup = FALSE since r_getrecord set the lock */
		e = st_deleterecord(filenum, &(dp->sptr[i].rid), 
			trans_id, FALSE, cond);
	    }
	    else 
	    {   /* it is a slice */
		/* discard the buffer and release the disk page */
		(void) bf_discard(filenum, &pid, spage);
		e = io_freepage(&fid, &pid);
	    }
	    if (e < eNOERROR) break;
	}

	/* unfix the buffer that contains the directory */
	(void) bf_freebuf(filenum, &(dirpage->thispage), dirpage);
	CHECKERROR(e);	/* any error so far? */

	/* remove the directory */
	/* lockup is set equal to FALSE since the directory is already locked */
	/* in exclusive mode */
	e = st_deleterecord(filenum, ridptr, trans_id, FALSE, cond); 
	return(e);

}	/* st_destroylong */

