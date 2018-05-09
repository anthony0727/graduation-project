
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
/* Module : am_fetchprev
      Fetch the previous RID that satisfies the search predicate of a scan.
  
   IMPORTS:
      int	st_prevfile(openfilenum, *prevrid, trans_id, 
    			lockup, mode, cond)
      int	st_getadjrid(fileno, *currRID, which, *key, *RID, trans_id,
			lockup, oper, cond)
      int	st_compare(fileno, *RID, operator, *keyattr, *value, 
			trans_id, lockup, cond)
      int	AM_apply(filno, *rid, *boolexpr)
       SCANINFO *AM_getscan();
  
   EXPORTS:
      int	am_fetchprev(ScanID, *PrevRID)
*/

#include    <wiss.h>
#include    <am.h>
  
extern    SCANINFO *AM_getscan();

int
am_fetchprev(scanid, prevrid,type)
int     scanid;	    /* ID of the scan in progress */
RID    *prevrid;	/* for returning the qualified previous RID */
enum logical_op	type;		/* AND, OR, NOT */

/* Fetch the previous RID that satisfies the search predicate of a scan.
      If this is an index scan, the records are also checked against the
      given lower bound.
  
   RETURNS:
      RID of the previous qualified record
  
   SIDE EFFECTS:
      None
  
   ERRORS:
      e3BADSCANTYPE - if scan type is not SEQUENTIAL or INDEXSCAN
*/

{
    register SCANINFO	*sptr;	    /* address of scan info record */
    register int	e;	    /* error return value */
    register int	filno;	    /* open file number */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    	printf("am_fetchprev(scanid=%d, prevrid=0x%x)\n",
    	     	scanid, prevrid);
#endif

    sptr = AM_getscan(scanid);	/* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

    filno = sptr->filenum;	/* file # of this file */
    switch (sptr->scantype)
    {
      case SEQUENTIAL:
    	do
    	{ /* loop until a qualified record is found */
    	    e = st_prevfile(filno, &sptr->rid, &sptr->rid, sptr->trans_id, 
		sptr->lockup, sptr->mode, sptr->cond);
    	    CHECKERROR(e);
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
        } /* end switch */

    	} while (e != TRUE);
    	break;

      case INDEXSCAN:
    	do
    	{ /* loop until a qualified record is found */
    	    e = st_getadjrid(sptr->indexfile, PREV, &(sptr->cursor), 
    	    	     &sptr->rid, sptr->trans_id, sptr->lockup, 
		     BT_READ, sptr->cond);
    	    CHECKERROR(e);
    	    if (sptr->lb != NULL)
    	    	if(st_compare(sptr->filenum, &sptr->rid, GE, sptr->keyattr, 
			(sptr->lb)->value, sptr->trans_id, 
			FALSE, sptr->cond) != TRUE)
    	    	    return(e3NOPREVRID);
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
        } /* end switch */

    	} while (e != TRUE);
    	break;
    	
      default:
    	return (e3BADSCANTYPE);	/* illegal for other types of scans */

    } /* end switch */

    if (prevrid != NULL)
    	*prevrid = sptr->rid;	/* return rid of previous record */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    {
    	printf("returning from am_fetchprev with prev RID = ");
    	PRINTRID(sptr->rid); printf("\n");
    }
#endif
    return (eNOERROR);

} /* am_fetchprev */
