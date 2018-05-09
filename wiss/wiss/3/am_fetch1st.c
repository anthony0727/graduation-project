
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
/* Module : am_fetchfirst
      Fetch the first RID that satisfies the search predicate of a scan. 
  
   IMPORTS:
      int	st_firstfile(openfilenum, *firstrid, trans_id, 
    	    		lockup, mode, cond)
      int	st_firstindex(openfilenum, *lowerbound, *cursor, *firstrid)
      int	st_gethash(openfilenum, *key, *cursor, *firstrid)
      int	st_compare(openfilenum, *RID, operator, *keyattr, *value, 
    	    		trans_id, lockup, cond);
      int	am_fetchnext(scanid, *rid)
      int	AM_apply(openfilenum, *rid, *boolexpr)
      SCANINFO *AM_getscan();
  
   EXPORTS:
      int	am_fetchfirst(ScanID, *FirstRID)
*/

#include    <wiss.h>
#include    <am.h>
#include        <lockquiz.h>


extern    SCANINFO *AM_getscan();

int
am_fetchfirst(scanid, firstrid, type)
int     scanid;	    /* Id of this scan */
RID     *firstrid;	/* for returning rid of the first qualified record */
enum    logical_op	type;		/* AND, OR, NOT */

/* Fetch the first RID that satisfies the search predicate of a scan. 
      If this is a index scan, the record is also checked
      against the upper bound given at the time the scan is opened.
  
   RETURNS:
      the RID of the first qualified record
      (if the return pointer is null, nothing will return and is not
       considered an error)
  
   SIDE EFFECTS:
      None
  
   ERRORS:
      e3BADSCANTYPE - if scan type is not INDEXSCAN or SEQUENTIAL
*/

{
    register SCANINFO	*sptr;	    /* address of scan info record */
    register int	e;	    /* error return value */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    	printf("am_fetchfirst(scanid=%d, ridptr=0x%x)\n", 
    	    	scanid, firstrid);
#endif
    sptr = AM_getscan(scanid);	/* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

    switch (sptr->scantype)
    {
      case SEQUENTIAL:
    	e = st_firstfile(sptr->filenum, &sptr->rid, sptr->trans_id, 
		sptr->lockup, sptr->mode, sptr->cond);
    	if (sptr->lockup) GETPID(sptr->locked_page, sptr->rid);
    	CHECKERROR(e);
	switch (type) {
		case AND :
			 e = AM_apply_and(sptr->filenum, &sptr->rid, sptr->boolexp);
        		break;
		case OR :
			 e = AM_apply_or(sptr->filenum, &sptr->rid, sptr->boolexp);
        	  	 break;
		case NOT :
    			e = AM_apply_not(sptr->filenum, &sptr->rid, sptr->boolexp);
    			break;
		case NONE:
				e = AM_apply_none(sptr->filenum, &sptr->rid, sptr->boolexp);
				break;
		default :
			return(-1);
	}  /* end of switch */
	break;

      case INDEXSCAN:
    	e = st_firstindex(sptr->indexfile, sptr->lb, &sptr->cursor, 
		&sptr->rid, sptr->trans_id, sptr->lockup, BT_READ, sptr->cond);
    	CHECKERROR(e);
    	if (sptr->ub == NULL) e = TRUE;
    	else e = st_compare(sptr->filenum, &sptr->rid, LE, sptr->keyattr, 
		      (sptr->ub)->value, sptr->trans_id, 
		      FALSE, sptr->cond); /* <= UB ? */
    	CHECKERROR(e);

/* dewitt&futter added the second clause to avoid an unnecessary call */
/* to AM_apply when the boolean expression is NULL */

    	if ((e == TRUE) && (sptr->boolexp != NULL)) {  /* satisfy predicate ? */ 

	 	switch (type) {
                	case AND :
                         	e = AM_apply_and(sptr->filenum, &sptr->rid,
					 sptr->boolexp);
                        	break;
                	case OR :
                         	e = AM_apply_or(sptr->filenum, &sptr->rid,
					 sptr->boolexp);
                         	break;
                	case NOT :
                        	e = AM_apply_not(sptr->filenum, &sptr->rid, 
					 sptr->boolexp);
                        	break;
					case NONE:
							e = AM_apply_none(sptr->filenum, &sptr->rid,
					sptr->boolexp);
							break;
                	default :
                        	return(-1);
        	}  /* end of switch */
	}
	break;

      case HASHSCAN:
    	e = st_gethash(sptr->indexfile, sptr->lb, 
    	    	&sptr->cursor, &sptr->rid, sptr->trans_id,
		sptr->lockup, sptr->cond);
    	CHECKERROR(e);

/* dewitt&futter added the following test to avoid an unnecessary call */
/* to AM_apply when the boolean expression is NULL */

    	if (sptr->boolexp != NULL) { /* satisfy predicate ? */
		switch (type) {
                        case AND :
                                e = AM_apply_and(sptr->filenum, &sptr->rid,
                                         sptr->boolexp);
                                break;
                        case OR :
                                e = AM_apply_or(sptr->filenum, &sptr->rid,
                                         sptr->boolexp);
                                break;
                        case NOT :
                                e = AM_apply_not(sptr->filenum, &sptr->rid,
                                         sptr->boolexp);
                                break;
						case NONE:
								e = AM_apply_none(sptr->filenum, &sptr->rid,
										sptr->boolexp);
								break;
                        default :
                                return(-1);
                }  /* end of switch */
        }
	break;

      default:
    	return(e3BADSCANTYPE);	/* illegal for other types of scans */
    }


    if (e != TRUE)
    {  /* first record not qualified */
    	e = am_fetchnext(scanid, (RID *) NULL, type);
    	CHECKERROR(e);
    }

    if (firstrid != NULL)
    	*firstrid = sptr->rid;	/* return RID of the first record */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    {
    	printf("returning from am_fetchfirst with first RID = ");
    	PRINTRID(sptr->rid); printf("\n");
    }
#endif

    return (eNOERROR);

}  /* am_fetchfirst */
