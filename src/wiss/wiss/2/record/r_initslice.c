
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
/* Module r_initslice : Level 2 internal routine to initialize a new slice

   IMPORTS:
	bf_getbuf(trans_id, filenum, fid, pageid, returnpage)

   EXPORTS:
	r_initslice(trans_id, filenum, pageid, returnpage)

*/

#include 	<wiss.h>
#include	<st.h>

r_initslice(filenum, pidptr, returnpage, trans_id)
int		trans_id;
int		filenum;	/* the file number */
PID		*pidptr;	/* the page the new slice will be on */
DATAPAGE	**returnpage;	/* place to return the buffer pointer */

/* Initialize a new slice. 

   Returns:
	(optionally) the page buffer for the slice 

   Side Effects:
	a buffer is allocated for the slice, and is FIXED in main memory

   Errors:
	None

*/

{
	int		e;		/* for returned errors */
	DATAPAGE	*pageptr;	/* pointer to a buffer */

#ifdef TRACE
	if (checkset(&Trace2, tSLICE)) {
		printf("r_initslice(filenum=", filenum);
		printf(",PID="); PRINTPIDPTR(pidptr); printf(")\n"); 
	}
#endif

	/* get a fresh buffer for the new slice */
	e = bf_getbuf(trans_id, filenum, FC_FILEID(filenum), pidptr, &pageptr);
	CHECKERROR(e);
	
	/* initial a record entry that takes up the entire page */
	((RECORD *) pageptr)->type = NOTMOVED;
	((RECORD *) pageptr)->kind = SLICE;
	((RECORD *) pageptr)->length = SLICESIZE;
	pageptr->fileid = F_FILEID(filenum);
	pageptr->thispage = *pidptr;
	pageptr->ridcnt = 1;
	pageptr->slot[0] = 0;
	pageptr->free = SLICESIZE + HEADERLEN;

	if (returnpage != NULL) *returnpage = pageptr; /* return the buffer */

	return(eNOERROR);

}	/* r_initslice */

