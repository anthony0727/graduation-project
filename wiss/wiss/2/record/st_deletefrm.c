
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
/* Module st_deleteframe : delete part of a long data item

   IMPORTS :
	io_freepage(fid, pid)
	bf_freebuf(filenum, pageid, pageptr)
	bf_setdirty(filenum, pageid, pageptr)
	bf_discard(filenum, pageid, pageptr)
	r_getslot(filenum, ridptr, page, recptr, trans_id, lockup, mode, cond)
	r_shrinkslice(filenum, ridptr, length, trans_id, lockup, cond)
	r_expandcrumb(filenum, ridptr, length, trans_id, lockup, cond)
	st_readrecord(filenum, ridptr, recaddr, len, trans_id, lockup, mode, cond)
	st_writerecord(filenum, ridptr, recaddr, len, trans_id, lockup, cond)

   EXPORTS :
	st_deleteframe(filenum, *rid, offset, length, trans_id, lockup, cond)

*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>


static delete_slices(filenum, dp, start, count)
int	filenum;	/* open file number */
LONGDIR	*dp;		/* pointer to the directory */
int	start;		/* where to start the deletion */
int	count;		/* # of slices to delete */

/*
Deletes "count" slices starting from slice "start".
This involves freeing pages and removing their entries from the directory.

Returns:
None

Side Effects:
Some pages may be freed as the slices on them are removed

Errors:
None
*/
{

	int		e;	/* for returned errors */
	register int	i; 	/* slice index */
	FID		fid;	/* level 0 file ID */
	PID		pid;	/* PID of the slice in concern */

#ifdef	TRACE
	if (checkset(&Trace2, tFRAME))
		printf("delete_slices(filenum=%d,dp=0x%x,start=%d,count=%d)\n",
		filenum, dp, start, count);
#endif

	if (count <= 0) return(eNOERROR);	/* no slice to be deleted */

	fid = F_FILEID(filenum);
	for (i = start; i < start + count; i++) {
#ifdef	DEBUG
		if (dp->sptr[i].rid.Rslot != 0) {
			printf("attempt to free a crumb page (s:%d) in deleteframe\n",
				dp->sptr[i].rid.Rslot);
			return(e2BADSLOTNUMBER);
		}
#endif
		GETPID(pid, dp->sptr[i].rid);
		e = io_freepage(&fid, &pid);
		CHECKERROR(e);
	}

	dp->slice_count -= count; 	/* decrement slice count */
	for (i = start; i < dp->slice_count; i++) /* remove directory entries */
		dp->sptr[i] = dp->sptr[i + count];

	return(eNOERROR);

}  /* delete_slices */


st_deleteframe(filenum, ridptr, offset, length, trans_id, lockup, cond)
int	filenum;	/* file that contains the long data item */
RID	*ridptr;	/* rid of the directory */
int	offset;		/* where to start */
int	length;		/* how much to remove */
int     trans_id;
short   lockup;
short   cond;

/*
  This deletes "length" bytes from a long data item starting from "offset".

  Returns:
	actual number of bytes deleted

  Side Effects:
	Some pages may be freed as the slice on them are removed.

  Errors:
	None
*/
{
	int		e;		/* for returned errors */
	register int	i, j;		/* slices indices */
	int		num_delete;	/* # of slices to be deleted */
	int		left;		/* # of bytes left to delete */
	char		*sptr, *sptr1;	/* slice buffer pointers */
	char		buf[PAGESIZE];	/* local directory buffer */
	register LONGDIR *dp=(LONGDIR *)buf;	/* direcotry pointer */
	RECORD		*recptr;	/* record pointer */
	DATAPAGE	*spi, *spj;	/* buffer pointers for slices */

#ifdef	TRACE
	if (checkset(&Trace2,tINTERFACE)) {
		printf("st_deleteframe(filenum=%d,RID=",filenum);
		PRINTRIDPTR(ridptr);
		printf(",offset=%d,length=%d)\n",offset,length);
	}
#endif

	/* check file number and file permission */
	CHECKOFN(filenum);
	CHECKWP(filenum);

	/* read the directory into a local buffer */
	e = st_readrecord(filenum, ridptr, (char *) dp, PAGESIZE, trans_id, lockup,
	    l_X, cond);
	CHECKERROR(e);

	if (offset < 0 || length <= 0) return(0);	/* do nothing */
	if (offset + length > dp->total_length)
		length = dp->total_length - offset;	/* trancate */
	if (length <= 0) return(0);	/* nothing to delete */

	/* locate the start of deletion - <slice i, offset> */
	for (i = 0; offset > dp->sptr[i].len; offset -= dp->sptr[i++].len);

	/* if the deletion starts from the trailing crumb ... */
	if (i == dp->slice_count-1 && dp->sptr[i].len <= CRUMBSIZE ) 
	{
	    RID	rid;	/* rid of the crumb */

	    /* update the directory */
	    left = dp->sptr[i].len;
	    rid = dp->sptr[i].rid;
	    dp->sptr[i].len -= length, dp->total_length -= length;
	    if (dp->sptr[i].len == 0) 
	    {
		dp->slice_count--; /* remove trailing empty crumb */
		while (((j=dp->slice_count-1) >= 0)&&(dp->sptr[j].len <= CRUMBSIZE))
		{
		    e = r_shrinkslice(filenum, &(dp->sptr[j].rid), 
		     dp->sptr[j].len, trans_id, lockup, cond);
		    CHECKERROR(e);
		    if (dp->sptr[j].len == 0) dp->slice_count--;
		    else break;
		}
	    }
	    e = st_writerecord(filenum, ridptr, (char *) dp, DIRLEN(dp), 
			trans_id, lockup, cond);
	    CHECKERROR(e);

	    /* read in the crumb and delete (reuse directory buffer) */
	    if (dp->sptr[i].len == 0) 
	    {   /* remove the crumb */
		e = st_deleterecord(filenum, &rid, trans_id, lockup, cond);
		CHECKERROR(e);
		return(length);
	    }
	    e = st_readrecord(filenum, &rid, buf, left, trans_id, 
		    lockup, l_X, cond);
	    CHECKERROR(e);
	    movebytes(buf+offset, buf+offset+length, left-offset);
	    e = st_writerecord(filenum, &rid, buf, left-length, trans_id, lockup,
			cond);
	    CHECKERROR(e);
	    return(length);		/* return actual # of bytes deleted */
	}

	/* locate the other end of deletion - <slice j, left> */
	left = length;
	if (dp->sptr[i].len >= offset + length) j = i;
	else for (left -= dp->sptr[i].len-offset, j = i+1; 
		left > dp->sptr[j].len; left -= dp->sptr[j++].len);
	if (j == dp->slice_count - 1) {
		/* expand the trailing crumb to ease further precessing */
		e = r_expandcrumb(filenum, &(dp->sptr[j].rid), dp->sptr[j].len,
			trans_id, lockup, cond);
		CHECKERROR(e);
	}

	if (i == j) 
	{
	    if (offset == 0 && length == dp->sptr[i].len)
			num_delete = 1;	/* need only to remove slice i */
	    else 
	    {   /* only slice i involved, no deletion of slices, if 
                offset+length==dp->sptr[i].len, no data be moved. */
		num_delete = 0;		
		if (offset < (dp->sptr[i].len -= length)) 
		{
		    e = r_getslot(filenum , &(dp->sptr[i].rid), &spi, &recptr, 
			trans_id, lockup, l_X, cond);
		    CHECKERROR(e);
		    sptr = recptr->data;
		    movebytes(sptr+offset, sptr+offset+length,
		    dp->sptr[i].len-offset); /* close gap */
		    (void) bf_setdirty(filenum,&spi->thispage,spi);
		    (void) bf_freebuf(filenum,&spi->thispage,spi);
	        }
	    }
	}
	else 
	{   /* more slices involved */
	    dp->sptr[i].len = offset;
	    num_delete = j - i - 1;	

	    /* At this point, <slice i, offset> & <slice j, left> are two ends of 
	     the deletion. If length of slice i and/or length of slice j are/is 
	     zero after the deletion, simply remove them/it without any data 
	     relocation; Otherwise, if slice i and j are small enough, merge them
	     into slice i and remove j.
	    */
	    if (left == dp->sptr[j].len) num_delete++; /* delete j too */ 
	    else 
	    {   /* merge slice i and j if possible , or shrink j */

		e = r_getslot(filenum, &dp->sptr[j].rid, &spj, &recptr,
			trans_id, lockup, l_X, cond);
		CHECKERROR(e);
		sptr1 = recptr->data;
		dp->sptr[j].len -= left;  /* # of bytes left in j */
		if (offset==0 || dp->sptr[i].len+dp->sptr[j].len>SLICESIZE) 
		{
			/* can't or no need to compress, shrink j */
			movebytes(sptr1, sptr1+left, dp->sptr[j].len);
			(void) bf_setdirty(filenum, &spj->thispage,spj);
			(void) bf_freebuf(filenum, &spj->thispage, spj);
		}
		else 
		{   /* merge slice i, j */ 
		    e = r_getslot(filenum, &dp->sptr[i].rid,&spi, &recptr, 
			    trans_id, lockup, l_X, cond);
		    if (e < eNOERROR) 
		    {
			(void) bf_freebuf(filenum, &(spj->thispage), spj);
			return(e);
		    }
		    sptr = recptr->data;
		    movebytes(sptr+offset,sptr1+left,dp->sptr[j].len);
		    dp->sptr[i].len += dp->sptr[j].len;
		    num_delete++;	/* delete j also */
		    (void) bf_setdirty(filenum, &spi->thispage,spi);
		    (void) bf_freebuf(filenum, &spi->thispage,spi);
		    (void) bf_discard(filenum, &spj->thispage,spj);
		}
	    }

	    if (offset == 0) num_delete++;	/* remove empty slice i */
	    else i++;	/* start removing from slice i+1 */
	}
	
	/* remove unneeded slices */
	e = delete_slices(filenum, dp, i, num_delete);
	CHECKERROR(e);

	/* adjust trailing crumb */
	while ((i=dp->slice_count-1) >= 0 && dp->sptr[i].len <= CRUMBSIZE) {
		e = r_shrinkslice(filenum, &(dp->sptr[i].rid), dp->sptr[i].len,
			trans_id, lockup, cond);
		CHECKERROR(e);
		if (dp->sptr[i].len == 0) dp->slice_count--;
		else break;
	}

	/* update directory */
	dp->total_length -= length;	/* update length in header */
	e = st_writerecord(filenum, ridptr, (char *) dp, DIRLEN(dp), 
		trans_id, lockup, cond);
	CHECKERROR(e);
	return(length);		/* return actual # of bytes deleted */

} /* st_deleteframe */


