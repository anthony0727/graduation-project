
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
/* Module : am_fetchlast
      Fetch the last RID that satisfies the search predicate of a scan.
  
   IMPORTS:
      int    st_lastfile(openfilenum, *lastrid, trans_id, 
                     lockup, mode, cond)
      int    st_lastindex(openfilenum, *upperbound, *cursor, *lastrid,
                     trans_id, lockup, oper, cond)
      int    st_compare(openfilenum, *RID, operator, *keyattr, 
                 *value, trans_id, lockup, cond);
      int    am_fetchprev(scanid, *rid)
      int    AM_apply(openfilenum, *rid, *boolexpr)
        SCANINFO *AM_getscan();
  
   EXPORTS:
      int    am_fetchlast(Scanid, lastrid)
*/

#include    <wiss.h>
#include    <am.h>
#include    <lockquiz.h>

  
extern    SCANINFO *AM_getscan();

int
am_fetchlast(scanid, lastrid, type)
int     scanid;        /* ID of the scan in progress */
RID     *lastrid;    /* for returning the last RID found */
enum logical_op	type;  /* AND, OR, NOT */

/* Fetch the last RID that satisfies the search predicate of a scan.
      If this is an index scan, the record is also checked against the
      given lower bound.
  
   RETURNS:
      RID of the last qualified record
  
   SIDE EFFECTS:
      none
  
   ERRORS:
      e3BADSCANTYPE - if scan type = LONGSCAN
*/

{
    register SCANINFO    *sptr;    /* address of scan info record */
    register     int e;        /* error return value */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
        printf("am_fetchlast(scanid=%d, lastrid=0x%x)\n",
                 scanid, lastrid);
#endif

    sptr = AM_getscan(scanid);    /* get address of scan info */
    if (sptr == NULL) return(e3BADSCANID);

    switch (sptr->scantype)
    {
      case SEQUENTIAL:
        e = st_lastfile(sptr->filenum, &sptr->rid, sptr->trans_id, 
		sptr->lockup, sptr->mode, sptr->cond);
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
	} /* end switch */
	break;

      case INDEXSCAN:
        switch(sptr->mode)
        {
	    case l_NL :
            case l_S : 
                e = st_lastindex(sptr->indexfile, sptr->ub, &sptr->cursor, 
			&sptr->rid, sptr->trans_id, sptr->lockup, BT_READ, 
			sptr->cond);
                break;
            case l_X : 
                e = st_lastindex(sptr->indexfile, sptr->ub, &sptr->cursor, 
			&sptr->rid, sptr->trans_id, sptr->lockup, BT_INSERT, 
			sptr->cond);
                break;
        }
        CHECKERROR(e);
        if (sptr->lb == NULL)
            e = TRUE;
        else
        {
	    /* >= LB ? */
            e = st_compare(sptr->filenum, &sptr->rid, GE, sptr->keyattr, 
		  (sptr->lb)->value, sptr->trans_id, FALSE, sptr->cond); 
            CHECKERROR(e);
        }
        if (e == TRUE) {
		switch(type) {
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
		}
	}

        break;

      default:
        return(e3BADSCANTYPE);    /* illegal for other types of scans */

    }

    if (e != TRUE)
    {  /* the last record not qualified */
        e = am_fetchprev(scanid, (RID *) NULL, type);
        CHECKERROR(e);
    }

    if (lastrid != NULL) *lastrid = sptr->rid; /* return the last RID */

#ifdef TRACE
    if (checkset (&Trace3, tINTERFACE))
    {
        printf("returning from am_fetchlast with last RID = ");
        PRINTRID(sptr->rid); printf("\n");
    }
#endif
    
    return (eNOERROR);

} /* am_fetchlast */
