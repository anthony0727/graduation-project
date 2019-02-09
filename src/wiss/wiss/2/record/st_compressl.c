
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
/* Module st_compresslong : compress a long data item

   IMPORTS :
	io_freepage(fid, pid)
	bf_freebuf(filenum, pageid, pageptr)
	bf_discard(filenum, pageid, pageptr)
	bf_setdirty(filenum, pageid, pageptr)
	r_getrecord(filenum, ridptr, page, recptr, trans_id, lockup, mode, cond)
	r_shrinkslice(filenum, ridptr, len, trans_id, lockup, cond)
	st_readrecord(filenum, rid, recaddr, len, trans_id, lockup, mode, cond)
	st_writerecord(filenum, rid, recaddr, len, trans_id, lockup, cond)
	st_deleterecord(filenum, ridptr, trans_id, lockup, cond);

   EXPORTS :
	st_compresslong(filenum, ridptr, pff, trans_id, lockup, cond)
*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


static compress_directory(filenum, dp, trans_id, lockup, cond)
int		filenum;	/* file number */
LONGDIR		*dp;		/* pointer to directory */
int     trans_id;
short   lockup;
short   cond;

/* This routine compresses the directory of a long data item.

Returns:
None

Side Effects:
empty slices are removed

Errors:
None
*/
{
	int		e;	/* for returned error codes */
	int		i, j;	/* slice indices */
	FID		fid;	/* level 0 file ID */
	PID		pid;	/* page ID of the slice to be removed */
	RID		srid;	/* RID of a slice */
	RECORD		*recptr; /* record pointer */
	DATAPAGE	*spage;	/* page buffer of a slice (or a crumb) */

#ifdef	TRACE
	if (checkset(&Trace2, tFRAME))
		printf("compress_directory(filenum=%d,dp=0x%x)\n", filenum, dp);
#endif

	/* locate the first empty slice */
	for (i = 0; (i < dp->slice_count) && (dp->sptr[i].len != 0); i++);
	if (i == dp->slice_count) return(eNOERROR);	/* nothing to do */

	/* In the following, i is the index of the current empty slice and
	j is used to locate the next non-empty slice.
	Empty slices encountered in each interation of the main loop are
	freed. The entry at j is then moved forward to i.
	*/
	fid = F_FILEID(filenum);	/* get internal file ID */
	for (j = i;; dp->sptr[i++] = dp->sptr[j++])
	{
		for (; dp->sptr[j].len == 0 && j < dp->slice_count; j++)
		{
			srid = dp->sptr[j].rid; /* read in the slice */
			e = r_getslot(filenum, &srid, &spage, &recptr,
				trans_id, lockup, l_X, cond);
			if (e < eNOERROR) break;
			GETPID(pid, srid);
			if (recptr->kind == CRUMB)
			{
				(void)bf_freebuf(filenum, &pid, spage);
				e = st_deleterecord(filenum, &srid, trans_id, lockup, cond);
			}
			else
			{ /* it is a slice */
#ifdef	DEBUG
				if (srid.Rslot != 0) {
					printf("bad slot # for a slice found in compresslong\n");
					e = e2BADSLOTNUMBER;
					break;
				}
#endif
				(void)bf_discard(filenum, &pid, spage);
				e = io_freepage(&fid, &pid);
			}
			if (e < eNOERROR) break;
		}
		if (j >= dp->slice_count || e < eNOERROR) break;
	}

	dp->slice_count = i;	/* # of slices left */

	return(e);

}  /* compress_directory */


st_compresslong(filenum, ridptr, pff, trans_id, lockup, cond)
int	filenum;	/* file number */
RID	*ridptr;	/* RID of the directory */
int	pff;		/* how full each slice should be */
int     trans_id;
short   lockup;
short   cond;

/* This routine compresses a long data item. 
   It is intended for the following occasions:
     1. at the end of editing a long data item,
     2. when free space on volume is insufficient for insertion.
   After compression, each slice, except the last, is quarrented to be at 
   least pff% full.

  Returns:
	None

  Side Effects:
	pages may be released as the long data item is compressed

  Errors:
	e2NULLRIDPTR:  pointer to the directory RID is null

  Special Notes:
	This routine may be very time-consuming if the long data item 
        is long, as it is supposed to be.
*/
{
	int		e;		/* for returned errors */
	register int	i, j;		/* slice indices */
	int		len;		/* # of bytes to move at a time */
	char		*sptr, *sptr1;	/* pointers into slice buffers */
	char		dbuf[PAGESIZE];	/* local directory buffer */
	LONGDIR	*dp = (LONGDIR *)dbuf;	/* pointer to directory */
	RECORD		*recptr;	/* record pointer */
	DATAPAGE	*spi, *spj;	/* buffer pointers to slices */

#ifdef	TRACE
	if (checkset(&Trace2,tINTERFACE)) {
		printf("st_compresslong(filenum=%d,RID=",filenum);
		PRINTRIDPTR(ridptr); printf("\n");
	}
#endif
	/* check file number and file permission */
	CHECKOFN(filenum);
	CHECKWP(filenum);

	if (ridptr == NULL) return(e2NULLRIDPTR);

	/* read in the directory of the long data item */
	e = st_readrecord(filenum, ridptr, (char *)dp, PAGESIZE, trans_id, 
		lockup, l_X, cond);
	CHECKERROR(e);

	if (dp->total_length == 0 || dp->slice_count <= 1) 
		goto compressed;	/* no need for slice compression */

	/* convert pff from % into bytes */
	if (pff <= 0 || pff > 100) pff = 100;
	pff = pff * SLICESIZE / 100;

	/* In the following, slice i is the end of the compressed portion 
	   and slice j is the next non-empty slice.
   	   In each iteration of the main loop, as much bytes as possible 
	   are moved from slice j to i. Empty slices are left IN PLACE and 
	   be removed later (to avoid extra data movement).
	*/
	for (i = j = 0; ;) {
		/* locate the next nonempty & nonfull slice i */
		for (; (dp->sptr[i].len == 0 || dp->sptr[i].len >= pff)
				&& i < dp->slice_count; i++);
		if (i >= dp->slice_count) break;  /* end of long item */

		/* locate the next nonempty slice j */
		for (j = i+1; dp->sptr[j].len == 0 && j < dp->slice_count; j++);
		if (j >= dp->slice_count) break; /* end of long item */

		/* retrieve both slices */
		e = r_getrecord(filenum, &(dp->sptr[i].rid), &spi, &recptr, 
		    trans_id, lockup, l_X, cond);
		CHECKERROR(e);
		sptr = recptr->data;

		e = r_getrecord(filenum, &(dp->sptr[j].rid), &spj, &recptr, 
		    trans_id, lockup, l_X, cond);
		if (e < eNOERROR) {
			(void) bf_freebuf(filenum, &(spi->thispage), spi);
			return(e);
		}
		sptr1 = recptr->data;

		/* move from slice j to i as mush as possible */
		len = (dp->sptr[i].len + dp->sptr[j].len <= SLICESIZE) ?
		   dp->sptr[j].len : pff-dp->sptr[i].len;
		movebytes(sptr+dp->sptr[i].len, sptr1, len);
		dp->sptr[i].len += len, dp->sptr[j].len -= len;

		/* unfix the buffers */
		(void) bf_setdirty(filenum, &(spi->thispage), spi);
		(void) bf_freebuf(filenum, &(spi->thispage), spi);
		if (dp->sptr[j].len > 0) /* close leading gap in slice j */
			movebytes(sptr1, sptr1 + len, dp->sptr[j].len);
		(void) bf_setdirty(filenum, &(spj->thispage), spj);
		(void) bf_freebuf(filenum, &(spj->thispage), spj);
	}

compressed:
	/* remove empty slices and compress the directory */
	e = compress_directory(filenum, dp, trans_id, lockup, cond);
	CHECKERROR(e);

 	/* shrink the last slice into a crumb if possible */
	if (dp->slice_count > 0) {
		e = r_shrinkslice(filenum, &(dp->sptr[dp->slice_count-1].rid), 
			dp->sptr[dp->slice_count-1].len, trans_id, lockup, cond);
		CHECKERROR(e);
	}

	/* update directory */
	e = st_writerecord(filenum, ridptr, (char *)dp, DIRLEN(dp), 
		trans_id, lockup, cond);
	return(e);

} /* st_compresslong */

