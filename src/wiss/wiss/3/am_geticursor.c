
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
/* Module : am_geticursor
  	Gets the cursor of an index scan.  This is for index scans ONLY.
  
   IMPORTS:
  	SCANINFO *AM_getscan(scanid)
  
   EXPORTS:
  	int	am_geticursor(scanid, *XCURSOR)
*/

#include	<wiss.h>
#include	<am.h>

extern	SCANINFO *AM_getscan();

int
am_geticursor(scanid, Xcursor)
int 	scanid;		/* ID of the scan */
XCURSOR *Xcursor;	/* the new cursor */

/* This returns the cursor of an index scan 
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	e3BADSCANTYPE - if type of this scanid is not INDEXSCAN.
*/
{
	register SCANINFO	*sptr;	/* address of scan info record */

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
		printf("am_geticursor(scanid=%d, *Xcursor0x%x)\n", 
			scanid, Xcursor);
#endif

	sptr = AM_getscan(scanid);	/* get address of scan info record */
	if (sptr == NULL) return(e3BADSCANID);

	if (sptr->scantype != INDEXSCAN) return (e3BADSCANTYPE);
		
	*Xcursor = sptr->cursor;	/* here is the cursor */

	return (eNOERROR);

} /* am_geticursor */
