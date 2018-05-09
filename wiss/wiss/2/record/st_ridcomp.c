
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



/********************************************************/
/*							*/
/*		Wisconsin Storage System		*/
/*		Version 2.0 July, 1984			*/
/*							*/
/*		COPYRIGHT (C) 1984			*/
/*		Computer Sciences Department		*/
/*		University of Wisconsin			*/
/*		Madison, WI 53706			*/
/*							*/
/********************************************************/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

/* Module st_ridcompare:  compare the fields of two records.  The
   two records may or may not be in the same file.  

   IMPORTS:
	r_getrecord(filenum, ridptr, pageptr, recptr)
	bf_freebuf(filenum, pageid, pagebuf)

   EXPORTS:
	st_ridcompare(ofn1, ridptr1, field1, ofn2, ridptr2, field2, op, 
		trans_id, lockup, cond)

*/

st_ridcompare(ofn1, ridptr1, field1, ofn2, ridptr2, field2, returnvalue,
	trans_id, lockup, cond)
int		ofn1;			/* open file number */
RID		*ridptr1;		/* pointers to RIDs of records */
FIELDDESC	*field1;		/* field information */
int		ofn2;			/* open file number */
RID		*ridptr2;		/* pointers to RIDs of records */
FIELDDESC	*field2;		/* field information */
int		*returnvalue;		/* pointer to return value */
int             trans_id;
short           lockup;
short           cond;
/*
   Returns:
	TRUE or FALSE

   Side Effects :
	NONE

   Errors :
	e2NULLRIDPTR
*/
{

	register char	*arg1, *arg2;	/* field pointers */
	register int length;	
	register char	*pz1, *pz2;	/* field pointers */
	int		e;		/* for returned error codes */
	double          z1, z2;
        double          rd;
        float           rf;
	long		rl;
	RECORD		*recptr1, *recptr2;	/* record pointers */
	DATAPAGE	*dp1 = NULL;
	DATAPAGE	*dp2 = NULL;	/* pointers to page buffers */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_ridcompare(ofn1=%d,", ofn1);
		printf("RID1="); PRINTRIDPTR(ridptr1);
		printf("field1 = "); PRINTATTR(field1);
		printf("st_ridcompare(ofn2=%d,", ofn2);
		printf("RID2="); PRINTRIDPTR(ridptr2);
		printf("field2 = "); PRINTATTR(field2); printf(")\n");
	}
#endif

	/* check input parameters */
	CHECKOFN(ofn1);
	CHECKOFN(ofn2);
	if (ridptr1 == NULL || ridptr2 == NULL) return(e2NULLRIDPTR);
	if (ridptr1 == ridptr2 && field1->offset == field2->offset) {
		*returnvalue = 0;
		return(eNOERROR);
	}

	/* get the address of the first record */
	e = r_getrecord(ofn1, ridptr1, &dp1, &recptr1, trans_id, 
		lockup, l_S, cond);
	CHECKERROR(e);
	arg1 = &(recptr1->data[field1->offset]);
        length = field1->length;  /* assume that the lengths are the same?? */

	/* Now, the second record */
	e = r_getrecord(ofn2, ridptr2, &dp2, &recptr2, trans_id,
		lockup, l_S,cond);
	if (e < eNOERROR) goto error;
	arg2 = &(recptr2->data[field2->offset]);

	/* we assume that the types are the same ??? */

	if ((field1->type != TSTRING) && (field1->type != TBITS))
	{ 
                /* copy arguments (which *might* not be aligned)
                        into local buffers */
                pz1 = (char *) (&z1); pz2 = (char *) (&z2); 
                for (; length > 0; length--) 
			*(pz1++) = *(arg1++), *(pz2++) = *(arg2++);
                arg1 = (char *) (&z1); arg2 = (char *) (&z2);
	}

	/* compare the fields */ 
	switch (field1->type) {
		case TINTEGER:
			*returnvalue = (*(int *)arg1 - *(int *)arg2);
			break;
		case TLONG:
			rl = *(long *)arg1 -  *(long *)arg2;
			*returnvalue = (rl == 0) ? 0 : (rl > 0) ? 1 : -1;
			break;
		case TSHORT:
			*returnvalue=  *(short *)arg1 -  *(short *)arg2;
			break;
		case TFLOAT:
			rf = *(float *)arg1 - *(float *)arg2;
			*returnvalue = (rf == 0) ? 0 : (rf > 0) ? 1 : -1;
			break;
		case TDOUBLE:
			rd = *(double *)arg1 - *(double *)arg2;
			*returnvalue = (rd == 0) ? 0 : (rd > 0) ? 1 : -1;
			break;
		case TSTRING:
			*returnvalue = strncmp(arg1, arg2, field1->length);
			break;
		case TBITS:
                        *returnvalue = tbitscmp(arg1, arg2, field1->length);
                        break;
		default:
			e = e2BADDATATYPE;
			goto error;

	} /* end switch */

	e = eNOERROR;
error:
	if (dp1 != NULL) 
	{
		(void) bf_freebuf(ofn1, &(dp1->thispage), dp1);
	}
	if (dp2 != NULL) 
	{
		(void) bf_freebuf(ofn2, &(dp2->thispage), dp2);
	}
	return(e);

} /* st_ridcompare */

