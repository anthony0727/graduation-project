
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
/* Module : am_setindexscan
  	Sets the cursor of an index scan to a given XCURSOR.
  	This is for index scans ONLY.

   IMPORTS:
  	SCANINFO *AM_getscan(scanid)
  
   EXPORTS:
  	int	am_setindexscan(scanid, *XCURSOR)
*/

#include	<wiss.h>
#include	<am.h>
  
extern	SCANINFO *AM_getscan();

int
am_setindexscan(scanid, NewXCURSOR)
int 	scanid;		/* ID of the scan */
XCURSOR *NewXCURSOR;	/* the new cursor */

/* This sets the cursor of an index scan to the given XCURSOR
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	the cursor (in the scan table) associated with this scan updated
  
   ERRORS:
  	e3BADSCANTYPE - if type of this scanid is not INDEXSCAN.
  
   BUGS:
  	The given XCURSOR should be checked to be sure it is associated
  	with the same file as the scan. This is not done since
  	it would be expensive to check. Furthermore, future use of any
  	invalid cursor will be caughted by other interface routines.
*/
{

	register SCANINFO	*sptr;	/* address of scan info record */

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
		printf("am_setindexscan(scanid=%d, *NewXCURSOR0x%x)\n", 
			scanid, NewXCURSOR);
#endif

	sptr = AM_getscan(scanid);	/* get address of scan info record */
	if (sptr == NULL) return(e3BADSCANID);

	if (sptr->scantype != INDEXSCAN) return (e3BADSCANTYPE);
		
	sptr->cursor = *NewXCURSOR;	/* set the cursor */

	return (eNOERROR);

} /* am_setindexscan */
