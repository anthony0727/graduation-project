
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

/* Module st_compare:  compare the field of a record with a given constant 

   IMPORTS:
	r_getrecord(filenum, ridptr, returnpage, recptr, transId, lockup, mode,
		cond)
	bf_freebuf(filenum, pageid, pagebuf)

   EXPORTS:
	st_compare(filenum, rid, relation, field, constant)

*/

st_compare(filenum, ridptr, relation, field, constant, trans_id, lockup, cond)
int		filenum;		/* open file number */
RID		*ridptr;		/* pointers to RIDs of records */
enum rel_op	relation;		/* comparison relation */
FIELDDESC	*field;			/* field information */
char		*constant;		/* the constant to be compared */
int             trans_id;
short           lockup;
short           cond;
/* Check if the field of a record satisfy a given relation with a constant

   Returns:
	TRUE or FALSE

   Side Effects :
	NONE

   Errors :
	NONE
*/
{
	register char	*k1, *k2;
	int		e;		/* for returned error codes */
	int		result;		/* result of comparison */
	DATAPAGE	*dp;		/* pointer to page buffer */
	RECORD		*recptr;	/* record pointer */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_compare(filenum=%d,", filenum);
		printf("RID="); PRINTRIDPTR(ridptr);
		printf("field="); PRINTATTR(field);
		printf(", constant=");
		print_data(field->type, field->length, constant); printf(")\n");
	}
#endif

	/* check the file number */
	CHECKOFN(filenum);

	/* locate the record */
	e = r_getrecord(filenum, ridptr, &dp, &recptr, trans_id, lockup, 
		l_S, cond);
	CHECKERROR(e);

	k1 = (char *) &(recptr->data[field->offset]);
	k2 = (char *) constant;

	result = compare_key(k1, k2, field->type, field->length);
	/* unfix the buffer */
	(void) bf_freebuf(filenum, &(dp->thispage), dp);
	if(result == e2BADDATATYPE)
		return	result;

	/* combine the result of comparison with the relational relation */
	switch (relation) {
		case EQ:
			return((result == 0) ? TRUE : FALSE);
		case NE:
			return((result != 0) ? TRUE : FALSE);
		case LT:
			return((result <  0) ? TRUE : FALSE);
		case LE:
			return((result <= 0) ? TRUE : FALSE);
		case GT:
			return((result >  0) ? TRUE : FALSE);
		case GE:
			return((result >= 0) ? TRUE : FALSE);
		default:
			return(e2ILLEGALOP);
	} /* end switch */

} /* st_compare */
