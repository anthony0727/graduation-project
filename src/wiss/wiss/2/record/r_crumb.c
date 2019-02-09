
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
/* Module r_crumb : Level 2 internal routines to convert a crumb into
	a slice or via versa.

   IMPORTS:
	io_freepage(fid, pid)
	io_allocpages(fid, nearpid, count, pidarray)
	bf_discard(filenum, pageid, pageptr)
	bf_freebuf(filenum, pageid, pageptr)
	r_initslice(filenum, pageid, returnpage, trans_id)	
	r_getslot(filenum, ridptr, returnpage, recptr, trans_id, lockup, mode, 
		cond)
	r_getrecord(filenum, ridptr, returnpage, recptr, trans_id, lockup, mode, 
		cond)
	st_appendrecord(filenum, recaddr, len, ridptr, trans_id, lockup, cond)
	st_deleterecord(filenum, ridptr, trans_id, lockup, cond)

   EXPORTS:
	r_expandcrumb(filenum, ridptr, length, trans_id, lockup, cond)
	r_shrinkslice(filenum, ridptr, length, trans_id, lockup, cond)
*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

r_shrinkslice(filenum, ridptr, length, trans_id, lockup, cond)
int		filenum;	/* file number */
RID		*ridptr;	/* RID of the slice */
int		length;		/* length of the slice */
int		trans_id;	/* transaction id */
short		lockup;	
short		cond;	

/* Given the RID of a slice, turn it into a crumb (special kind of record)
   If the slice is empty, then remove it completely.

   Returns:
	the RID of the crumb (via ridptr)

   Side Effects:
	None

   Errors:
	e2NULLRIDPTR - null RID pointer
*/
{
	int		e;		/* for returned errors */
	FID		fid;		/* level 0 file id */
	PID		pid;		/* level 0 pid of the slice */
	RECORD		*recptr;	/* record pointer */
	DATAPAGE	*pageptr;	/* pointer to the page buffer */

#ifdef TRACE
	if (checkset(&Trace2, tSLICE)) {
		printf("r_shrinkslice(filenum=%d%,RID=", filenum);
		PRINTRIDPTR(ridptr);
		printf(",slice size=%d)\n", length); 
	}
#endif

	if (ridptr == NULL) 
		return(e2NULLRIDPTR); 	/* null RID pointer */
	if (length > CRUMBSIZE)
		return(eNOERROR);	/* too large to be a crumb */

	e = r_getrecord(filenum, ridptr, &pageptr, &recptr, trans_id, lockup, 
		l_X, cond);
	CHECKERROR(e);
	if (recptr->kind == CRUMB) {
		(void) bf_freebuf(filenum, &(pageptr->thispage), pageptr);
		if (length == 0) e = st_deleterecord(filenum, ridptr, trans_id, 
				lockup, cond);
		else e = eNOERROR;
		return(e);	/* already a crumb ! */
	}

	/* get level 1 info of the slice */
	fid = pageptr->fileid;
	pid = pageptr->thispage;

	if (length == 0) { /* remove the slice completely */
		/* free the page and the buffer the slice was on */
		(void) bf_discard(filenum, &(pageptr->thispage), pageptr);
		e = io_freepage(&fid, &pid);
		return(e);
	}

	/* create a crumb as a record */
	e = st_appendrecord(filenum, recptr->data, length, ridptr, trans_id,
		lockup, cond);
	if (e < eNOERROR) {
		(void) bf_freebuf(filenum, &(pageptr->thispage), pageptr);
		return(e);	/* something wrong */
	}

	/* free the page and the buffer the slice was on */
	(void) bf_discard(filenum, &(pageptr->thispage), pageptr);
	e = io_freepage(&fid, &pid);
	CHECKERROR(e);

/* SHERROR, no solution yet, passing dummy variabes to avoid locking */
	/* mark the record as a crumb */
	e = r_getslot(filenum, ridptr, &pageptr, &recptr, 
		trans_id, lockup, l_X, cond);
	CHECKERROR(e);
	recptr->kind = CRUMB;
	(void) bf_setdirty(filenum, &(pageptr->thispage), pageptr);
	(void) bf_freebuf(filenum, &(pageptr->thispage), pageptr);
	
	return(eNOERROR);

}	/* r_shrinkslice */


r_expandcrumb(filenum, ridptr, length, trans_id, lockup, cond)
int		filenum;	/* file number */
RID		*ridptr;	/* RID of the crumb */
int		length;		/* length of the crumb */
int     	trans_id;
short   	lockup;
short   	cond;


/* Given RID of a crumb, turn it into a slice

   Returns:
	the RID of the slice (via ridptr)

   Side Effects:
	None

   Errors:
	e2NULLRIDPTR - null RID pointer
*/
{
	int		e;		/* for returned errors */
	FID		fid;		/* level 0 file id */
	PID		pid;		/* level 0 pid of the slice */
	RID		crumbrid;	/* RID of the crumb */
	RECORD		*recptr;	/* record pointer */
	DATAPAGE	*cpage, *spage;	/* pointers to page buffers */

#ifdef TRACE
	if (checkset(&Trace2, tSLICE)) {
		printf("r_expandcrumb(filenum=%d%,RID=", filenum);
		PRINTRIDPTR(ridptr);
		printf(",slice size=%d)\n", length); 
	}
#endif

	if (ridptr == NULL) return(e2NULLRIDPTR);	/* null RID pointer */

	/* get the crumb */
	crumbrid = *ridptr;
	e = r_getrecord(filenum, &crumbrid, &cpage, &recptr, 
		trans_id, lockup, l_X, cond);
	CHECKERROR(e);
	if (recptr->kind != CRUMB) { /* unfix the buffer and return */
		(void) bf_freebuf(filenum, &(cpage->thispage), cpage);
		return(eNOERROR);	/* it is not a crumb ! */
	}

	/* allocate a disk page for the new slice and initialize it */
	fid = F_FILEID(filenum);
	e = io_allocpages(&fid, (PID *)NULL, 1, &pid);
	if (e < eNOERROR) { 
		(void) bf_freebuf(filenum, &(cpage->thispage), cpage);
		return(e);
	}
	e = r_initslice(filenum, &pid, &spage, trans_id);
	if (e < eNOERROR) {
		(void) bf_freebuf(filenum, &(cpage->thispage), cpage);
		return(e);
	}
	MAKERID(ridptr, &pid, 0); /* return the RID of the slice */

	/* copy the data from the crumb to the new slice and unfix buffers */
	movebytes(((RECORD *)spage)->data, recptr->data, length);
	(void) bf_freebuf(filenum, &(cpage->thispage), cpage);
	(void) bf_freebuf(filenum, &(spage->thispage), spage);

	/* delete the old crumb */
	e = st_deleterecord(filenum, &crumbrid, trans_id, lockup, cond);
	CHECKERROR(e);

	return(eNOERROR);

}	/* r_expandcrumb */

