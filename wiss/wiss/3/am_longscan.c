
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
/* Module : am_openlongscan
  	Opens a scan on a long data item.
  
   IMPORTS:
  	int	 AM_addscan(OpenFileNum)     
  	SCANINFO *AM_getscan(scanID)
  
   EXPORT:
  	int	am_openlongscan(openfilenum, *rid, trans_id, mode, lockup, cond)
*/

#include <wiss.h>
#include <am.h>
#include        <lockquiz.h>

     
extern	SCANINFO *AM_getscan();

int
am_openlongscan (openfilenum, rid, trans_id, lockup, mode, cond)
int	 openfilenum;	/* id of the open file */
RID	 *rid;		/* rid for the long data item (i.e. long directory) */
int	 trans_id;
short	 lockup;	/* true if locking is to be turned on */
LOCKTYPE mode;          /* lock mode */
short    cond;          /* true if conditional locking is to be used */

/* Opens a scan on a long data item.
  
   Returns:
  	scan id associated with the scan.
     
   Side effects: 
  	A new entry is added to the scan table
  
   Errors:
  	None
*/
{
	register SCANINFO *sptr;	/* pointer to scan info record */
	register int 	scanid ;	/* scaned associated with new scan */
	
#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
	{
		printf("am_openlongscan(openfilenum=%d, rid=", openfilenum);
		PRINTRID(*rid); printf(")\n");
	}
#endif

	/* add a new scan to the open scan table */
	scanid = AM_addscan(openfilenum);
	CHECKERROR(scanid);

	/* get the address of the scan info record */
	sptr = AM_getscan(scanid);
	if (sptr == NULL) return(e3BADSCANID);

	/* fill in the scan info for a long item scan */
	sptr->scantype = LONGSCAN;
	sptr->offset = 0;
	sptr->dirty_bit = FALSE;
	sptr->rid = *rid;
	sptr->trans_id = trans_id;
	sptr->lockup = lockup;
	sptr->cond = cond;
	sptr->mode = mode;

	return (scanid);

} /* am_openlongscan */
