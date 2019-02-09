
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



/*  File Header 
 *
 * Imports (functions) :
 *	bf_freebuf(filenum, pageid, pageptr)
 *	r_getrecord(filenum, ridptr, page, recptr, trans_id, lockup, mode, cond)
 *
 * Imports (variables) :
 *
 * Exports (functions) :
 *	st_readframe(filenum, rid, offset, trans_id, lockup, cond) 
 *
 * Exports (variables) :
 */

/*  HISTORY 
 *
 */

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

/*  Function Header 
 *
 * Name         : st_readlength
 * Purpose      : Given the RID of a long item directory, read 
 *		  bytes of data from the long data item until
 *		  meeting a null character (end of string)
 * Arguments    : 
 *		  int	filenum : open file number
 *		  RID	*ridptr : RID of the directory
 *		  int	offset : where to start
 *
 * Return value : a length (positive integer) or e2NULLRIDPTR (if 
 *		  pointer to the directory RID is null)
 *
 * Side effects : none
 * Comments     : extracted from st_readframe
 *
 */

st_readlength(filenum, ridptr, offset, trans_id, lockup, cond)
     int	filenum;	/* open file number */
     RID	*ridptr;	/* RID of the directory */
     int	offset;		/* where to start */
     int	trans_id;
     short	lockup;
     short	cond;
{
  int		e;	/* for returned error codes */
  int		i;	/* slice index */
  int		slen;	/* # of bytes to read in current slice */
  int		slen_tmp;	/* to modify slen temporary */
  int		count;	/* # of bytes left to be read */
  char		*sptr;	/* slice buffer pointer */
  LONGDIR	*dp;	/* directory pointer */
  RECORD	*recptr;	/* record pointer */
  DATAPAGE	*dirpage;	/* page pointer */
  DATAPAGE	*spage;		/* page pointer */
  int		strlength= 0;	/* the calculated string length to return) */
  char		*cptr;		/* to go along a string */
				   
#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_readlength(filenum=%d,RID=", filenum);
		PRINTRIDPTR(ridptr);
		printf(",offset=%d)\n",offset);
	}
#endif

	/* check the file number */
	CHECKOFN(filenum);

	if (ridptr == NULL) return(e2NULLRIDPTR);

	/* read in the directory of the long data item */
	e = r_getrecord(filenum, ridptr, &dirpage, &recptr, 
			trans_id, lockup, l_S, cond);
	CHECKERROR(e);
	dp = (LONGDIR *) recptr->data;

	/* check the requested length against the length of the actual data */
	if (offset >= dp->total_length || offset < 0) {
		/* unfix the buffer that contains the directory */
		(void) bf_freebuf(filenum, &(dirpage->thispage), dirpage);
		return(0);	 /* offset out of bound or length negative */
	}

	/* locate the first slice to read, and the offset into that slice */
	for (i = 0; offset >= dp->sptr[i].len; offset -= dp->sptr[i++].len);

	/* In each iteration of the following loop, "slen" bytes (at most) 
	   from slice i are examined. "Sptr" points to the slice buffer. 
	*/
	for (count = dp->total_length - offset; count > 0; i++, count -= slen) { 
		for(; dp->sptr[i].len == 0; i++);	/* skip empty slices */
		e = r_getrecord(filenum, &(dp->sptr[i].rid), &spage, &recptr, 
				trans_id, lockup, l_S, cond);
		if (e < eNOERROR) break;
		sptr = recptr->data;
		slen = MIN(count, dp->sptr[i].len - offset);
		/* increases strlength until finding a null character or the 
		   end of the slice */
		for (cptr= sptr+offset, slen_tmp= slen; 
		     slen_tmp-- && *cptr; cptr++,strlength++);

		(void) bf_freebuf(filenum, &(spage->thispage), spage);
		offset = 0;   /* offset is zero after the 1st slice is read */
	}

	/* unfix the buffer that contains the directory */
	(void) bf_freebuf(filenum, &(dirpage->thispage), dirpage);

	return(strlength);	/* return actual # of bytes read */

}	/* st_readlength */

