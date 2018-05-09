
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
/* Module : AM_boolean
        This module evaluates Boolean expressions, returning TRUE if the
   	expression is true for the record being tested and FALSE otherwise.
	This version combines st_compare.c with AM_apply.
  
   IMPORTS:
  	st_compare(ofn, *RID, operator, *fielddesc, value, trans_id, 
		lockup, cond)
  
   EXPORTS:
  	int	AM_apply(open file number, *RID, *boolean_expression)
  	int	AM_dumpboolean(*boolean_expression)

*/

#include	<wiss.h>
#include	<am.h>
#include	<st_r.h>
#include 	<lockquiz.h>

int
AM_apply_and(ofn, RIDptr, Bool_ptr)
int	ofn;			/* open file number */
RID	*RIDptr;		/* pointer to RID of record to be tested */
BOOLEXP	*Bool_ptr;		/* pointer to Boolean expression */

/* This routine evaluates a Boolean expression, returning TRUE if the
    expression is true for the record being tested and FALSE if the expression
    is false.  It is passed an open file number, a pointer to a RID, and a
    pointer to a Boolean expression.
  
        The Boolean expression consists of a linked list of boolean terms
    connected (implicitly) by ANDs. Each term contains a field's offset from 
    the beginning of the record, a relational operator, a value against which 
    the field beginning at the given offset is compared, a data type (same for 
    both field and argument), and a pointer to the next term. If the routine is 
    passed a NULL expression pointer, it returns TRUE; that is, an empty Boolean
    expression always evaluates to TRUE
  
   RETURNS:
  	TRUE	if the record satisfies the given boolean expression
  	FALSE	otherwise
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	none
*/
{

	register int	result;		/* boolean result of comparison */

	/* following declarations are from st_compare.c */
	enum rel_op	relation;		/* comparison relation */
	FIELDDESC	*field;			/* field information */
	char		*constant;		/* the constant to be compared */
	int		e;		/* for returned error codes */
	register char	*arg1;		/* field pointer */
	DATAPAGE	*dp;		/* pointer to page buffer */
	RECORD		*recptr;	/* record pointer */
	float		rf;
	double		rd;
	char		buf1[sizeof(double)];	/* local field buffers */

#ifdef TRACE
	if (checkset(&Trace3, tBOOLEAN))
	{
		printf("AM_apply(ofn=%d, RID=", ofn); PRINTRIDPTR(RIDptr);
		printf(", boolean expression = "); AM_dumpboolean(Bool_ptr);
		printf(")\n");
	}
#endif
	/* locate the record */
	e = r_getrecord(ofn, RIDptr, &dp, &recptr, -1, FALSE, l_NL, FALSE);
	CHECKERROR(e);

	/* loop through the chain of subexpressions */
	for (; Bool_ptr != NULL; Bool_ptr = 
		(BOOLEXP *) ((char *) Bool_ptr + Bool_ptr->next) )
	{
		field = &Bool_ptr->fielddesc;
		arg1 = &(recptr->data[field->offset]);

		/* if field type is not a string and is aligned on
		odd address (lsb of the address is a 1), then load
		the field into a local buffer */

		if ( ((int) arg1 & 1) && field->type != TSTRING) { 
			movebytes(buf1, arg1, field->length);
			arg1 = buf1;
		}
	
		/* compare the field in the record with the given constant */
		switch (field->type) {
		case TINTEGER:
			result = *(int *)arg1 - *(int *)Bool_ptr->value;
			break;
		case TLONG:
			result = *(long *)arg1 -  *(long *)Bool_ptr->value;
			break;
		case TSHORT:
			result =  *(short *)arg1 -  *(short *)Bool_ptr->value;
			break;
		case TFLOAT:
			rf = *(float *)arg1 - *(float *)Bool_ptr->value;
			result = (rf == 0.0) ? 0 : (rf > 0.0) ? 1 : -1;
			break;
		case TDOUBLE:
			rd = *(double *)arg1 - *(double *)Bool_ptr->value;
			result = (rd == 0.0) ? 0 : (rd > 0.0) ? 1 : -1;
			break;
		case TSTRING:
			result = strncmp(arg1, Bool_ptr->value, field->length);
			break;
		default:
			F_LASTPINNED(ofn) = FALSE;
			(void) bf_freebuf(ofn, &(dp->thispage), dp);
			return(e2BADDATATYPE);

		} /* end switch */


		/* combine result of comparison with the relational operator */
		switch (Bool_ptr->roperator) {
			case EQ:
				if (result == 0)  result = TRUE; 
				else result = FALSE;
				break;
			case NE:
				if (result != 0) result = TRUE;
				else result = FALSE;
				break;
			case LT:
				if (result < 0) result = TRUE;
				else result = FALSE;
				break;
			case LE:
				if (result <= 0) result = TRUE;
				else result = FALSE;
				break;
			case GT:
				if (result > 0) result = TRUE;
				else result = FALSE;
				break;
			case GE:
				if (result >= 0) result = TRUE;
				else result = FALSE;
				break;
			default:
				return(e2ILLEGALOP);
		} /* end switch */

		CHECKERROR(result);
		if (result == FALSE) 
		{
			/* unfix the buffer */
			F_LASTPINNED(ofn) = FALSE;
			(void) bf_freebuf(ofn, &(dp->thispage), dp);
			CHECKERROR(e);
			return(FALSE);	/* this term failed */
		}
		if (Bool_ptr->next == NULL) break;
	}

	/* at this point, all subexpressions have evaluated to true, so
	 * the whole expression must be true */

	/* unfix the buffer */
	F_LASTPINNED(ofn) = FALSE;
	(void) bf_freebuf(ofn, &(dp->thispage), dp);
	CHECKERROR(e);

	return (TRUE);

} /* AM_apply_and */


int
AM_dumpboolean(boolptr)
BOOLEXP	*boolptr;		/* pointer to a boolean expression */

/* This routine dumps a boolean expression term by terms.
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	None
*/
{

	/* the following operator names dependent on the order of the 
		enumeration type for comparison operators */
	static	char	*opname[] = {"=", "<>", "<", "<=", ">", ">="};

	if (boolptr == NULL) printf(" null expression");
	 
	for (; boolptr != NULL; boolptr = (BOOLEXP *) 
		((char *) boolptr + boolptr->next) )
	{
		printf("{offset[%d]",boolptr->fielddesc.offset);
		printf(" %s ", opname[(int) boolptr->roperator]);
		print_data(boolptr->fielddesc.type,
			boolptr->fielddesc.length,boolptr->value);
		printf("}");
		if (boolptr->next != NULL) printf(" ^ ");
		else break;
	}

} /* AM_dumpboolean */

int
AM_apply_or(ofn, RIDptr, Bool_ptr)
int	ofn;			/* open file number */
RID	*RIDptr;		/* pointer to RID of record to be tested */
BOOLEXP	*Bool_ptr;		/* pointer to Boolean expression */

/* This routine evaluates a Boolean expression, returning TRUE if the
    one of any expression is true for the record being tested and FALSE
    if the all of the expression
    is false.  It is passed an open file number, a pointer to a RID, and a
    pointer to a Boolean expression.
  
	Commented by LSM

        The Boolean expression consists of a linked list of boolean terms
    connected (implicitly) by ORs. Each term contains a field's offset from 
    the beginning of the record, a relational operator, a value against which 
    the field beginning at the given offset is compared, a data type (same for 
    both field and argument), and a pointer to the next term. If the routine is 
    passed a NULL expression pointer, it returns TRUE; that is, an empty Boolean
    expression always evaluates to TRUE
  
   RETURNS:
  	TRUE	if the record satisfies the given boolean expression
  	FALSE	otherwise
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	none
*/
{

	register int	result;		/* boolean result of comparison */

	/* following declarations are from st_compare.c */
	enum rel_op	relation;		/* comparison relation */
	FIELDDESC	*field;			/* field information */
	char		*constant;		/* the constant to be compared */
	int		e;		/* for returned error codes */
	register char	*arg1;		/* field pointer */
	DATAPAGE	*dp;		/* pointer to page buffer */
	RECORD		*recptr;	/* record pointer */
	float		rf;
	double		rd;
	char		buf1[sizeof(double)];	/* local field buffers */

#ifdef TRACE
	if (checkset(&Trace3, tBOOLEAN))
	{
		printf("AM_apply(ofn=%d, RID=", ofn); PRINTRIDPTR(RIDptr);
		printf(", boolean expression = "); AM_dumpboolean(Bool_ptr);
		printf(")\n");
	}
#endif
	/* locate the record */
	e = r_getrecord(ofn, RIDptr, &dp, &recptr, -1, FALSE, l_NL, FALSE);
	CHECKERROR(e);

	/* loop through the chain of subexpressions */
	for (; Bool_ptr != NULL; Bool_ptr = 
		(BOOLEXP *) ((char *) Bool_ptr + Bool_ptr->next) )
	{
		field = &Bool_ptr->fielddesc;
		arg1 = &(recptr->data[field->offset]);

		/* if field type is not a string and is aligned on
		odd address (lsb of the address is a 1), then load
		the field into a local buffer */

		if ( ((int) arg1 & 1) && field->type != TSTRING) { 
			movebytes(buf1, arg1, field->length);
			arg1 = buf1;
		}
	
		/* compare the field in the record with the given constant */
		switch (field->type) {
		case TINTEGER:
			result = *(int *)arg1 - *(int *)Bool_ptr->value;
			break;
		case TLONG:
			result = *(long *)arg1 -  *(long *)Bool_ptr->value;
			break;
		case TSHORT:
			result =  *(short *)arg1 -  *(short *)Bool_ptr->value;
			break;
		case TFLOAT:
			rf = *(float *)arg1 - *(float *)Bool_ptr->value;
			result = (rf == 0.0) ? 0 : (rf > 0.0) ? 1 : -1;
			break;
		case TDOUBLE:
			rd = *(double *)arg1 - *(double *)Bool_ptr->value;
			result = (rd == 0.0) ? 0 : (rd > 0.0) ? 1 : -1;
			break;
		case TSTRING:
			result = strncmp(arg1, Bool_ptr->value, field->length);
			break;
		default:
			F_LASTPINNED(ofn) = FALSE;
			(void) bf_freebuf(ofn, &(dp->thispage), dp);
			return(e2BADDATATYPE);

		} /* end switch */


		/* combine result of comparison with the relational operator */
		switch (Bool_ptr->roperator) {
			case EQ:
				if (result == 0)  result = TRUE; 
				else result = FALSE;
				break;
			case NE:
				if (result != 0) result = TRUE;
				else result = FALSE;
				break;
			case LT:
				if (result < 0) result = TRUE;
				else result = FALSE;
				break;
			case LE:
				if (result <= 0) result = TRUE;
				else result = FALSE;
				break;
			case GT:
				if (result > 0) result = TRUE;
				else result = FALSE;
				break;
			case GE:
				if (result >= 0) result = TRUE;
				else result = FALSE;
				break;
			default:
				return(e2ILLEGALOP);
		} /* end switch */

		CHECKERROR(result);
		if (result == TRUE) 
		{
			/* unfix the buffer */
			F_LASTPINNED(ofn) = FALSE;
			(void) bf_freebuf(ofn, &(dp->thispage), dp);
			CHECKERROR(e);
			return(TRUE);	/* this term failed */
		}
		if (Bool_ptr->next == NULL) break;
	}

	/* at this point, all subexpressions have evaluated to true, so
	 * the whole expression must be true */

	/* unfix the buffer */
	F_LASTPINNED(ofn) = FALSE;
	(void) bf_freebuf(ofn, &(dp->thispage), dp);
	CHECKERROR(e);

	return (FALSE);

} /* AM_apply_or */


int
AM_apply_not(ofn, RIDptr, Bool_ptr)
int	ofn;			/* open file number */
RID	*RIDptr;		/* pointer to RID of record to be tested */
BOOLEXP	*Bool_ptr;		/* pointer to Boolean expression */

/* This routine evaluates a Boolean expression, returning FALSE if the
    expression is true for the record being tested and TRUE
    if the expression
is false.  It is passed an open file number, a pointer to a RID, and a
    pointer to a Boolean expression.
  
	NOTE : this function is called only with A boolean expression. If there
	exists list of boolean expresions, first of boolean expression is tested.
	
	COMMENTED by LSM
 
        The Boolean expression consists of a  boolean term
     The term contains a field's offset from 
    the beginning of the record, a relational operator, a value against which 
    the field beginning at the given offset is compared, a data type (same for 
    both field and argument), and a pointer to the next term. If the routine is 
    passed a NULL expression pointer, it returns FALSE; that is, an empty Boolean
    expression always evaluates to FALSE
  
   RETURNS:
  	TRUE	if the record does not satisfy the given boolean expression
  	FALSE	otherwise
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	none
*/
{

	register int	result;		/* boolean result of comparison */

	/* following declarations are from st_compare.c */
	enum rel_op	relation;		/* comparison relation */
	FIELDDESC	*field;			/* field information */
	char		*constant;		/* the constant to be compared */
	int		e;		/* for returned error codes */
	register char	*arg1;		/* field pointer */
	DATAPAGE	*dp;		/* pointer to page buffer */
	RECORD		*recptr;	/* record pointer */
	float		rf;
	double		rd;
	char		buf1[sizeof(double)];	/* local field buffers */

#ifdef TRACE
	if (checkset(&Trace3, tBOOLEAN))
	{
		printf("AM_apply_not(ofn=%d, RID=", ofn); PRINTRIDPTR(RIDptr);
		printf(", boolean expression = "); AM_dumpboolean(Bool_ptr);
		printf(")\n");
	}
#endif
	/* locate the record */
	e = r_getrecord(ofn, RIDptr, &dp, &recptr, -1, FALSE, l_NL, FALSE);
	CHECKERROR(e);

for (; Bool_ptr != NULL; Bool_ptr = 
		(BOOLEXP *) ((char *) Bool_ptr + Bool_ptr->next)){

	field = &Bool_ptr->fielddesc;
	arg1 = &(recptr->data[field->offset]);

	/* if field type is not a string and is aligned on
	odd address (lsb of the address is a 1), then load
	the field into a local buffer */

	if ( ((int) arg1 & 1) && field->type != TSTRING) { 
		movebytes(buf1, arg1, field->length);
		arg1 = buf1;
	}
	
	/* compare the field in the record with the given constant */
	switch (field->type) {
	case TINTEGER:
		result = *(int *)arg1 - *(int *)Bool_ptr->value;
		break;
	case TLONG:
		result = *(long *)arg1 -  *(long *)Bool_ptr->value;
		break;
	case TSHORT:
		result =  *(short *)arg1 -  *(short *)Bool_ptr->value;
		break;
	case TFLOAT:
		rf = *(float *)arg1 - *(float *)Bool_ptr->value;
		result = (rf == 0.0) ? 0 : (rf > 0.0) ? 1 : -1;
		break;
	case TDOUBLE:
		rd = *(double *)arg1 - *(double *)Bool_ptr->value;
		result = (rd == 0.0) ? 0 : (rd > 0.0) ? 1 : -1;
		break;
	case TSTRING:
		result = strncmp(arg1, Bool_ptr->value, field->length);
		break;
	default:
		F_LASTPINNED(ofn) = FALSE;
		(void) bf_freebuf(ofn, &(dp->thispage), dp);
		return(e2BADDATATYPE);

	} /* end switch */


	/* combine result of comparison with the relational operator */
	switch (Bool_ptr->roperator) {
		case EQ:
			if (result == 0)  result = TRUE; 
			else result = FALSE;
			break;
		case NE:
			if (result != 0) result = TRUE;
			else result = FALSE;
			break;
		case LT:
			if (result < 0) result = TRUE;
			else result = FALSE;
			break;
		case LE:
			if (result <= 0) result = TRUE;
			else result = FALSE;
			break;
		case GT:
			if (result > 0) result = TRUE;
			else result = FALSE;
			break;
		case GE:
			if (result >= 0) result = TRUE;
			else result = FALSE;
			break;
		default:
			return(e2ILLEGALOP);
	} /* end switch */
	CHECKERROR(result);
	if (result == FALSE) 
	{
		/* unfix the buffer */
		F_LASTPINNED(ofn) = FALSE;
		(void) bf_freebuf(ofn, &(dp->thispage), dp);
		CHECKERROR(e);
		return(TRUE);
	}
	if (Bool_ptr->next == NULL) break;

	}

	/* at this point, the expression has evaluated to true */

	/* unfix the buffer */
	F_LASTPINNED(ofn) = FALSE;
	(void) bf_freebuf(ofn, &(dp->thispage), dp);
	CHECKERROR(e);

	return (FALSE);

} /* AM_apply_not */


int
AM_apply_none(ofn, RIDptr, Bool_ptr)
int	ofn;			/* open file number */
RID	*RIDptr;		/* pointer to RID of record to be tested */
BOOLEXP	*Bool_ptr;		/* pointer to Boolean expression */
{
	/* following declarations are from st_compare.c */
	enum rel_op	relation;		/* comparison relation */
	int		e;		/* for returned error codes */
	DATAPAGE	*dp;		/* pointer to page buffer */
	RECORD		*recptr;	/* record pointer */

#ifdef TRACE
	if (checkset(&Trace3, tBOOLEAN))
	{
		printf("AM_apply_not(ofn=%d, RID=", ofn); PRINTRIDPTR(RIDptr);
		printf(", boolean expression = "); AM_dumpboolean(Bool_ptr);
		printf(")\n");
	}
#endif
	/* locate the record */
	e = r_getrecord(ofn, RIDptr, &dp, &recptr, -1, FALSE, l_NL, FALSE);
	CHECKERROR(e);

	/* unfix the buffer */
	F_LASTPINNED(ofn) = FALSE;
	(void)bf_freebuf(ofn, &(dp->thispage), dp);
	CHECKERROR(e);

	return (TRUE);

} /* AM_apply_none */