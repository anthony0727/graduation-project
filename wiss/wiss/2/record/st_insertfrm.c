
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
/* Module st_insertframe : insert a string of bytes into a long data item

   IMPORTS :
	io_allocpages(fid, nearpid, numpages, pidarray)
	bf_freebuf(filenum, pageid, pageptr)
	bf_setdirty(filenum, pageid, pageptr)
	r_initslice(filenum, pageid, returnpage, trans_id)
	r_getslot(filenum, ridptr, page, recptr, trans_id, lockup, mode, cond)
	r_shrinkslice(filenum, ridptr, length, trans_id, lockup, cond)
	r_expandcrumb(filenum, ridptr, length, trans_id, lockup, cond)
	st_readrecord(filenum, ridptr, recaddr, len, trans_id, lockup, 
		mode, cond)
	st_writerecord(filenum, ridptr, recaddr, len, trans_id, lockup, cond)

   EXPORTS :
	st_insertframe(filenum, ridptr, offset, recaddr, length, trans_id, 
		lockup, cond)
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>


static make_room(trans_id, filenum, dp, start, len, pff)
int		trans_id;
int		filenum;	/* file number */
register LONGDIR *dp;		/* pointer to directory */
TWO		start;		/* which slice to start */
int		len;		/* # of bytes to insert */
int		pff;		/* page fill factor in bytes */

/* This routine makes room for a long data item to grow.
Slice "start" is the first slice affected by the insertion.
If all the new stuff can fit into slice "start", the routine simply
returns; otherwise it allocates new slices (pages) and add the corresponding
RID-count pair into the directory. No actual data movement is done here.

Returns:
index of the last new slice

Side Effects:
pages may be allocated for the new slices

Errors:
e2NOMORESLICES: long item grows too long
*/
{

	int		e;		/* for returned errors */
	register int	i, j;		/* slice indices */
	register int	count;		/* # of new slices needed */
	DATAPAGE	*sp;		/* page buffer for a slice */
	FID		fid;		/* level 0 file ID */
	PID		pids[MAXRECORDSIZE / ((int) sizeof(SLICEPTR))];	/* for new pids */

#ifdef	TRACE
	if (checkset(&Trace2, tFRAME))
		printf("make_room(filenum=%d,dp=0x%x,start=%d,len=%d,pff=%d)\n",
		filenum, dp, start, len, pff);
#endif

	if (dp->slice_count == 0) start = -1;		/* a bran new item */
	else if (dp->sptr[start].len + len <= pff)
		return(start); 			/* all the new bytes fit */
	else if (pff > dp->sptr[start].len)
		len -= pff - dp->sptr[start].len; /* use up the space */

	/* determine the number of new slices needed */
	count = (len % pff) ? len / pff + 1 : len / pff;
	if (DIRLEN(dp) + ((int) sizeof(SLICEPTR)) * count > MAXRECORDSIZE)
		return(e2NOMORESLICES);	/* directory too big */
	j = start + count;	/* postion of the last empty slice */

	/* reuse existing empty slices and allocates new disk pages */
	for (i = start + 1; i<dp->slice_count && dp->sptr[i].len == 0; i++)
		if (--count == 0) return(j);	/* enough empty slices */
	fid = F_FILEID(filenum);	/* get internal file ID */
	e = io_allocpages(&fid, (PID *)NULL, count, &pids[start + 1]);
	CHECKERROR(e);

	/* make room in the directory for the new entries */
	for (i = dp->slice_count - 1; i>start; i--) dp->sptr[i + count] = dp->sptr[i];

	/* set up control information on slice pages: the length of each slice
	is set to SLICESIZE to ensure that it is the only record on that page */
	for (i = start + 1; i <= start + count; i++) {
		e = r_initslice(filenum, &pids[i], &sp, trans_id);
		CHECKERROR(e);
		(void)bf_freebuf(filenum, &(pids[i]), sp);
		MAKERID(&(dp->sptr[i].rid), &(pids[i]), 0);
		dp->sptr[i].len = 0;
	}
	dp->slice_count += count;	/* the new slice count */

	return(j);	/* return index of the last empty slice */

}  /* make_room */


st_insertframe(filenum, ridptr, offset, recaddr, length, trans_id, lockup, cond)
int	filenum;	/* file that contains the long data item */
RID	*ridptr;	/* rid of the directory */
int	offset;		/* where to start */
char	*recaddr;	/* from where */
int	length;		/* how much to insert */
int     trans_id;
short   lockup;
short   cond;


/*
  Inserts "length" of bytes into a long data item.
  This routine attempts to reduce total data movement to the minimum and 
  keep each slice as full as possible at the same time.

  Returns:
	actual number of bytes inserted

  Side Effects:
	new pages may be allocated

  Errors:
	e2NULLRIDPTR: null RID pointer for the directory
*/
{
	int		e;			/* for returned errors */
	register int	i, j;			/* slice indices */
	int		pff;			/* page fill factor in bytes */
	int		left;			/* # of bytes left to insert */
	int		last_slice;		/* last slice of insertion */
	int		high_mark;		/* rear end of insertion */
	int		moves;			/* # of bytes to be moved */
	char		*sptr, *sptr1;		/* slice buffer pointers */
	RECORD		*recptr;		/* record pointer */
	char		buf[PAGESIZE];		/* local directory buffer */
	register LONGDIR *dp=(LONGDIR *)buf;	/* directory pointer */
	DATAPAGE	*spi, *spj;		/* buffer pointers */

#ifdef	TRACE
	if (checkset(&Trace2,tINTERFACE)) {
		printf("st_insertframe(filenum=%d,RID=",filenum);
		PRINTRIDPTR(ridptr);
		printf(",offset=%d,length=%d)\n",offset,length);
	}
#endif
		/* check file number and file permission */
	CHECKOFN(filenum);
	CHECKWP(filenum);

	if (ridptr == NULL) return(e2NULLRIDPTR);

	/* read the directory into a local buffer */
	e = st_readrecord(filenum, ridptr, (char *) dp, PAGESIZE, trans_id, 
		lockup, l_X, cond);
	CHECKERROR(e);

	/* check length info and get page fill factor */
	left = length;
	if (offset < 0 || length <= 0) return(0);	/* do nothing */
	if (offset > dp->total_length) return(0);	/* bad offset */
	pff = F_PFF(filenum);		/* page fill factor in % */
	pff = SLICESIZE * pff / 100;	/* page fill factor in bytes */

	/* If this is a new long item which can fit into a crumb, then
	   create a new record (a crumb) for the new bytes and return.
	*/ 
	if (dp->slice_count == 0 && left <= CRUMBSIZE) 
	{
		/* put the new bytes into a new crumb */
		e = st_appendrecord(filenum, recaddr, left, &(dp->sptr[0].rid), 
			trans_id, lockup, cond);
		CHECKERROR(e);
		e = r_getslot(filenum, &(dp->sptr[0].rid),&spi,&recptr, 
			trans_id, lockup, l_X, cond);;
		CHECKERROR(e);
		recptr->kind = CRUMB;	/* mark the record as a crumb */
		(void) bf_setdirty(filenum, &(spi->thispage), spi);

		(void) bf_freebuf(filenum, &(spi->thispage), spi);

		/* update the directory and return */
		dp->slice_count = 1;
		dp->sptr[0].len = dp->total_length = left;
		/* lockup is set to FALSE as r_getslot left it locked in X mode */
		e = st_writerecord(filenum,ridptr,(char *)dp,DIRLEN(dp), 
			trans_id, FALSE, cond);
		CHECKERROR(e);
		return(length);		/* return actual # of bytes inserted */
	}

	/* locate the start of insertion - (slice i, offset)  */
	for (i = 0; offset > dp->sptr[i].len && i < dp->slice_count;
						 offset -= dp->sptr[i++].len);

	/* If the new bytes are to be inserted into a trailing crumb ... */
	if (i == dp->slice_count-1 && dp->sptr[i].len + left <= CRUMBSIZE) 
	{
		RID	rid;	/* rid of the crumb */

		/* update the directory and write it out */
		moves = dp->sptr[i].len;
		dp->sptr[i].len += length, dp->total_length += length;

		/* lockup is set to FALSE as st_readrecord locked in X mode */
		e = st_writerecord(filenum, ridptr, (char *) dp, DIRLEN(dp), 
			trans_id, FALSE, cond);
		CHECKERROR(e);

		/* get the crumb and insert new bytes (reuse directory buffer) */
		rid = dp->sptr[i].rid;
		e = st_readrecord(filenum, &rid, buf, moves, 
			trans_id, lockup, l_X, cond);
		CHECKERROR(e);
		movebytes(buf+offset+length, buf+offset, moves-offset);
		movebytes(buf+offset, recaddr, length); /* new bytes */
		/* lockup is set to FALSE as st_readrecord locked in X mode */
		e = st_writerecord(filenum, &rid, buf, moves+length, 
			trans_id, FALSE, cond);
		CHECKERROR(e);
		return(length);		/* return actual # of bytes inserted */
	}

	/* If the trailing crumb is affected by the insertion, expand it into
	   a slice for the ease of further precessing.
	*/
	if (dp->slice_count > 0 && i >= dp->slice_count - 2) {
		e = r_expandcrumb(filenum, &(dp->sptr[dp->slice_count-1].rid), 
			dp->sptr[dp->slice_count-1].len, trans_id, lockup, cond);
		CHECKERROR(e);
	}

	/* allocate pages for the new slices */
	j = last_slice = make_room(trans_id, filenum, dp, i, left, pff);
	CHECKERROR(j);

	/* get the first slice involved and start the processing */
	e = r_getslot(filenum, &(dp->sptr[i].rid), &spi, &recptr, 
		trans_id, lockup, l_X, cond);
	CHECKERROR(e);
	sptr = recptr->data;
	if (i == j) {	/* can handle insertion in slice i */
		movebytes(sptr+offset+left, sptr+offset, 
		     dp->sptr[i].len-offset);  /* make room for new bytes */
		movebytes(sptr+offset, recaddr, left);	/* insert */
		dp->sptr[i].len += left;
		(void) bf_setdirty(filenum, &(spi->thispage), spi);
		(void) bf_freebuf(filenum, &(spi->thispage), spi);
	}
	else if (j == i + 1) { /* Only slices i and i+1 are affected. */
	/* If pff<offset, leave the part before "offset" in slice i alone */
		/* get slice j & move bottom half of slice i to j */

		e = r_getslot(filenum, &(dp->sptr[j].rid), &spj, &recptr, 
			trans_id, lockup, l_X, cond);
		if (e < eNOERROR) goto error;
		sptr1 = recptr->data;
		moves = dp->sptr[i].len - offset;  
		dp->sptr[j].len = (offset > pff) ? left+moves : 
		    (left < pff-offset) ? moves : left+moves - (pff-offset);
		movebytes(sptr1+dp->sptr[j].len-moves, sptr+offset,moves);

		/* if there is still room on slice i, put some new bytes there &
		   put the rest of the new bytes on slice j */
		if ((moves = MIN(pff-offset, left)) > 0) {	
			movebytes(sptr+offset, recaddr, moves);
			left -= moves, recaddr += moves;
		}
		movebytes(sptr1, recaddr, left);
		dp->sptr[i].len = (pff > offset) ? offset + moves : offset;

		/* mark the buffers dirty and unfix them */
		(void) bf_setdirty(filenum, &(spi->thispage), spi);
		(void) bf_setdirty(filenum, &(spj->thispage), spj);
		(void) bf_freebuf(filenum, &(spi->thispage), spi);
		(void) bf_freebuf(filenum, &(spj->thispage), spj);
	}
	else {	/* more slices are involved */
	/* In the following, insertion is done backward, slice by slice.
	   At any time, <slice i, offset> is the (fixed) front end of insertion,
	   and <slice j, high_mark> is the running rear end. 
	   After the insertion, the last newly added slice and its neighbor will
	   be checked for possible merging.
	   Every new slice, except the last, will contain exactly "pff" bytes. 
	   Slice i may contain more if offset > pff.
	*/
		/* locate the rear end <slice j, high_mark> */
		moves = (offset > pff) ? left+dp->sptr[i].len-offset 
					: left-(pff-dp->sptr[i].len);
		high_mark = moves % pff;
		if (i + moves / pff >= j) high_mark+=pff; /* fold back to j */
		dp->sptr[j].len = high_mark;

		/* move the bottom part of slice i to j as much as possible */

		e = r_getslot(filenum, &(dp->sptr[j].rid), &spj, &recptr, 
			trans_id, lockup, l_X, cond);;
		if (e < eNOERROR) goto error;
		sptr1 = recptr->data;
		moves = MIN(high_mark, dp->sptr[i].len - offset);
		dp->sptr[i].len -= moves;	/* what left in i */
		movebytes(sptr1+high_mark-moves, sptr+dp->sptr[i].len, moves);
			
		/* if more stuff in slice i need to be moved (to j-1) ... */
		if ((high_mark -= moves) == 0) { 
			(void) bf_setdirty(filenum, &(spj->thispage), spj);
			(void) bf_freebuf(filenum, &(spj->thispage), spj);
			moves = dp->sptr[i].len - offset;
			j--;	/* back up one slice */
			dp->sptr[j].len = pff;	

			e = r_getslot(filenum, &(dp->sptr[j].rid),&spj,&recptr,
			    trans_id, lockup, l_X, cond);;
			if (e < eNOERROR) goto error;
			sptr1 = recptr->data;
			dp->sptr[i].len -= moves;
			high_mark = pff - moves;
			movebytes(sptr1+high_mark, sptr+offset, moves);
		}

 		/* main loop for backward insertion */
		for (recaddr += left-high_mark; i < j ; recaddr -= high_mark) { 
			/* copy data into slice j */
			movebytes(sptr1, recaddr, high_mark);
			(void) bf_setdirty(filenum, &(spj->thispage), spj);
			(void) bf_freebuf(filenum, &(spj->thispage), spj);

			high_mark = pff ;/* every slice is full from now on */
			if (--j > i) { /* get the previous slice */
				dp->sptr[j].len = pff;
				e = r_getslot(filenum, &(dp->sptr[j].rid), &spj, 
					&recptr, trans_id, lockup, l_X, cond);;
				if (e < eNOERROR) goto error;
				sptr1 = recptr->data;
			} 
			else { /* alas! the last slice */
				if (offset < pff) { 
					moves = high_mark - offset;
					movebytes(sptr+offset,
						recaddr-moves,moves);
					dp->sptr[i].len = pff;
				}
				(void) bf_setdirty(filenum,&spi->thispage, spi);
				(void) bf_freebuf(filenum,&spi->thispage, spi);
			}
		}
	
		/* check if slices at the rear end of insertion mergable */
		i = last_slice;
		j = i+1;
		if (j<dp->slice_count &&
				dp->sptr[i].len+dp->sptr[j].len<=SLICESIZE) {
			e = r_getslot(filenum, &(dp->sptr[i].rid),&spi,&recptr,
				trans_id, lockup, l_X, cond);
			CHECKERROR(e);
			sptr = recptr->data;
			e = r_getslot(filenum, &(dp->sptr[j].rid),&spj,&recptr,
				trans_id, lockup, l_X, cond);
			if (e < eNOERROR) goto error;
			sptr1 = recptr->data;
			movebytes(sptr+dp->sptr[i].len, sptr1, dp->sptr[j].len);
			dp->sptr[i].len += dp->sptr[j].len;
			dp->sptr[j].len = 0; /* leave it in place */
			(void) bf_setdirty(filenum, &(spi->thispage), spi);
			(void) bf_freebuf(filenum, &(spi->thispage), spi);
			(void) bf_freebuf(filenum, &(spj->thispage), spj);
		}
	}
	
	/* trailing slice is too small to be a slice, turn it into a crumb */
	while ((i=dp->slice_count-1) >= 0 && dp->sptr[i].len <= CRUMBSIZE) {
		e = r_shrinkslice(filenum, &(dp->sptr[i].rid), dp->sptr[i].len,
			trans_id, lockup, cond);
		CHECKERROR(e);
		if (dp->sptr[i].len == 0) dp->slice_count--;
		else break;
	}

	dp->total_length += length;
	/* lockup is set to false as st_readrecord alreaded locked the directory */
	/* in X mode */
	e = st_writerecord(filenum, ridptr, (char *) dp, DIRLEN(dp), 
		trans_id, FALSE, cond);
	CHECKERROR(e);
	return(length);		/* return actual # of bytes inserted */

error:
	(void) bf_freebuf(filenum, &(spi->thispage), spi); /* unfix the buffer */
	return(e);	/* return the error code */

} /* st_insertframe */


#ifdef	DEBUG
static print_dir(dp)
LONGDIR	*dp;
{
	int	i;
	RID	r;

	printf("Diretory of the long data item: \n");
	for (i = 0; i < dp->slice_count; i++) {
		r = dp->sptr[i].rid;
		printf("slice[%2d]=<%d:%d>,len=%2d; ", 
			i, r.Rpage, r.Rslot, dp->sptr[i].len);
		if (i % 3 == 2) printf("\n");
	}
	printf("\n");
}
#endif

