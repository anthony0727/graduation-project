
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
/* Module st_readframe : routine to read part of a long data item

   IMPORTS :
	bf_freebuf(filenum, pageid, pageptr)
	r_getrecord(filenum, ridptr, page, recptr, trans_id, lockup, mode, cond)

   EXPORTS :
	st_readframe(filenum, rid, offset, recaddr, len, trans_id, lockup, cond) 
*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


st_readframe(filenum, ridptr, offset, recaddr, len, trans_id, lockup, cond)
int	filenum;	/* open file number */
RID	*ridptr;	/* RID of the directory */
int	offset;		/* where to start */
char	*recaddr;	/* record buffer address */
int	len;		/* number of bytes to read */
int     trans_id;
short   lockup;
short   cond;


/* Given the RID of a long item directory, read "len" bytes of data from
   the long data item into the buffer pointed by recaddr. 

   Returns:
	Number of bytes actually read

   Side Effects:
	None

   Errors:
	e2NULLRIDPTR:  pointer to the directory RID is null
*/
{
	int		e;	/* for returned error codes */
	int		i;	/* slice index */
	int		slen;	/* # of bytes to read in current slice */
	int		count;	/* # of bytes left to be read */
	char		*sptr;	/* slice buffer pointer */
	LONGDIR		*dp;	/* directory pointer */
	RECORD		*recptr;	/* record pointer */
	DATAPAGE	*dirpage;	/* page pointer */
	DATAPAGE	*spage;		/* page pointer */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_readframe(filenum=%d,RID=", filenum);
		PRINTRIDPTR(ridptr);
		printf(",offset=%d,recaddr=0x%x,len=%d)\n",offset,recaddr,len);
	}
#endif

	/* check the file number */
	CHECKOFN(filenum);

	if (ridptr == NULL) return(e2NULLRIDPTR);

	/* read in the directory of the long data item */
	e = r_getrecord(filenum, ridptr, &dirpage, &recptr, trans_id, lockup, 
		l_S, cond);
	CHECKERROR(e);
	dp = (LONGDIR *) recptr->data;

	/* check the requested length against the length of the actual data */
	if (offset >= dp->total_length || offset < 0 || len < 0) {
		/* unfix the buffer that contains the directory */
		(void) bf_freebuf(filenum, &(dirpage->thispage), dirpage);
		return(0);	 /* offset out of bound or length negative */
	}
	if (dp->total_length < len + offset)	
		len = dp->total_length - offset;	/* trancate */

	/* locate the first slice to read, and the offset into that slice */
	for (i = 0; offset >= dp->sptr[i].len; offset -= dp->sptr[i++].len);

	/* In each iteration of the following loop, "slen" bytes are read 
	   from slice i to "recaddr". "Sptr" points to the slice buffer. 
	*/
	for (count = len; count > 0; i++, count -= slen) { 
		for(; dp->sptr[i].len == 0; i++);	/* skip empty slices */
		e = r_getrecord(filenum, &(dp->sptr[i].rid), &spage, &recptr, 
			trans_id, lockup, l_S, cond);
		if (e < eNOERROR) break;
		sptr = recptr->data;
		slen = MIN(count, dp->sptr[i].len - offset);
		movebytes(recaddr, sptr+offset, slen);
		(void) bf_freebuf(filenum, &(spage->thispage), spage);
		offset = 0;   /* offset is zero after the 1st slice is read */
		recaddr += slen;
	}

	/* unfix the buffer that contains the directory */
	(void) bf_freebuf(filenum, &(dirpage->thispage), dirpage);

	return(len-count);	/* return actual # of bytes read */

}	/* st_readframe */

