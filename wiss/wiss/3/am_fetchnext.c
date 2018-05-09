
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
/* Module : am_fetchnext
      Fetch the next RID that satisfies the search predicate of a scan.
  
   IMPORTS:
      int	st_nextfile(filno, *nextrid, trans_id, lockup, mode, cond)
      int	st_getadjrid(fileno, *currRID, which, *key, *RID, trans_id,
			lockup, oper, cond)
      int	st_nexthash(fileno, Xcursor, *RID, trans_id, lockup, 
			mode, cond)
      int	st_compare(fileno, *RID, operator, *keyattr, *value, 
    			trans_id, lockup, cond)
      int	AM_apply(filno, *rid, *boolexpr)
      SCANINFO *AM_getscan(scanid)
  
   EXPORTS:
      int	am_fetchnext(ScanID, *NextRID)
*/

#include    <wiss.h>
#include    <am.h>
  
extern    SCANINFO *AM_getscan();

int
am_fetchnext(scanid, nextrid, type)
int     scanid;	    /* ID of the scan in progress */
RID     *nextrid;	/* for returning the next qualified RID */
enum logical_op	type;   /* AND, OR, NOT */

/* Fetch the next RID that satisfies the search predicate of a scan.
      
   RETURNS:
      RID of the next qualified record
  
   SIDE EFFECTS: 
   None 

   ERRORS:
      e3BADSCANTYPE - if scan type is not INDEXSCAN or SEQUENTIAL
*/

{
    register SCANINFO	*sptr;	    /* pointer to scan info record */
    register int	e;	    /* error return value */
    register int	filno;	    /* open file number */
    
#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    	printf("am_fetchnext(scanid=%d,nextrid=0x%x)\n", 
    	    scanid, nextrid);
#endif

    sptr = AM_getscan(scanid);	/* get address of scan info address */
    if (sptr == NULL) return(e3BADSCANID);

    filno = sptr->filenum;	/* file # of this file */

    switch (sptr->scantype)
    {
      case SEQUENTIAL:
    	do
    	{ /* loop until a qualified record is found */
    	    e = st_nextfile(filno, &sptr->rid, &sptr->rid, sptr->trans_id, 
		sptr->lockup, sptr->mode, sptr->cond);
    	    if (sptr->lockup) GETPID(sptr->locked_page,sptr->rid);
    	    CHECKERROR(e);
    	    if (sptr->boolexp == NULL)
    	    	break;	/* unconditional fetch next */
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
    	    e = st_getadjrid(sptr->indexfile, NEXT, &(sptr->cursor), 
		&sptr->rid, sptr->trans_id, sptr->lockup, BT_READ, sptr->cond);
    	    CHECKERROR(e);
    	    if (sptr->ub != NULL)
	    {
    	    	if(st_compare(sptr->filenum, &sptr->rid, LE, sptr->keyattr, 
		  (sptr->ub)->value, sptr->trans_id, FALSE, sptr->cond) != TRUE)
    	    	    		return(e3NONEXTRID);
	    }
    	    if (sptr->boolexp == NULL) break;	/* unconditional fetch next */
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

      case HASHSCAN:
    	do
    	{ /* loop until a qualified record is found */
    	    e = st_nexthash(sptr->indexfile, 
    	    	&(sptr->cursor), &sptr->rid, sptr->trans_id,
		sptr->lockup, sptr->cond);
    	    CHECKERROR(e);
    	    if (sptr->boolexp == NULL)
    	    	break;	/* unconditional fetch next */
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
    	return (e3BADSCANTYPE);

    } /* end switch */

    if (nextrid != NULL)
    	*nextrid = sptr->rid;	/* return RID of next record */


#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    {
    	printf("returning from am_fetchnext with next RID = ");
    	PRINTRID(sptr->rid); printf("\n");
    }
#endif

    return (eNOERROR);

} /* am_fetchnext */
