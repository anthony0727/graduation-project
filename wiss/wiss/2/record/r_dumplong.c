
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
/* Module r_dumplong : routine to dump a long data item

   IMPORTS :
	bf_freebuf(filenum, pageid, pageptr)
	r_getrecord(filenum, ridptr, page, recptr, trans_id, lockup, mode, cond)

   EXPORTS :
	r_dumplong(filenum, ridptr, trans_id, lockup, cond) 

*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

#define	WIDTH	60	/* line width for dump */

r_dumplong(filenum, ridptr, trans_id, lockup, cond)
int     filenum;        /* open file number */
RID     *ridptr;        /* RID of the directory */
int     trans_id;
short   lockup;
short   cond;

/* 
   Given the RID of a long item directory, dump the whole long data item.
   This routine is intended for debugging purposes.

   Returns:
	None

   Side Effects:
	None

   Errors:
	None
*/
{
	int		e;		/* for returned errors */
	int		i, j, k;	/* indices */
	int		count;		/* byte count */
	char		*sptr;		/* pointer into a slice */
	char		linebuf[WIDTH+4];/* buffer for output */
	RID		srid;		/* RID of the current slice */
	LONGDIR		*dp;		/* directory pointer */
	RECORD		*recptr;	/* record pointer */
	DATAPAGE	*dirpage;	/* page pointer */
	DATAPAGE	*spage;		/* page pointer */

#ifdef TRACE
	if (checkset(&Trace2, tFRAME)) {
		printf("r_dumplong(filenum=%d,RID=", filenum);
		PRINTRIDPTR(ridptr); printf(")\n");
	}
#endif

	/* read in the directory of the long data item */
	e = r_getrecord(filenum, ridptr, &dirpage, &recptr, trans_id, lockup,
		l_S, cond);
	CHECKERROR(e);
	dp = (LONGDIR *) recptr->data;

printf("\n=================================================================\n");
printf(" LONG DATA ITEM (total length = %d, slice count = %d)\n",
					dp->total_length, dp->slice_count);

	for (i = 0; i < dp->slice_count; i++) { 

		/* fetch the slice */
		srid = dp->sptr[i].rid, count = dp->sptr[i].len;

		e = r_getrecord(filenum, &srid, &spage, &recptr, 
			trans_id, lockup, l_S, cond);
		if (e < eNOERROR) break; 	/* something went wrong */
		sptr = recptr->data;

		/* print the slice */
		printf("%s %2d : RID=%2d:%2d:%2d, Length=%d\n",
			(recptr->kind==CRUMB)? "Crumb" : "Slice", i,
			srid.Rvolid, srid.Rpage, srid.Rslot, count);
		for (; count > 0; count -= WIDTH, sptr+= WIDTH) {
			j = MIN(count, WIDTH);
			for (k = 0; k < j; linebuf[k] = sptr[k], k++);
			linebuf[j] = '\0';
			printf("%-60.60s\n", linebuf);
		}

		/* unfix its buffer */
		(void) bf_freebuf(filenum, &(spage->thispage), spage);
	}

printf("=================================================================\n\n");

	/* unfix the buffer that contains the directory */
	(void) bf_freebuf(filenum, &(dirpage->thispage), dirpage);

	return(e);

}	/* r_dumplong */

