
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
/* Module : am_setscan
  	Sets the cursor of a sequential scan to a given RID.
  	This is for sequential scans ONLY.

   IMPORTS:
  	SCANINFO *AM_getscan(scanid)
  
   EXPORTS:
  	int	am_setscan(scanid, *RID)
*/
  
#include	<wiss.h>
#include	<am.h>

extern	SCANINFO *AM_getscan();

int
am_setscan(scanid, NewRID)
int 	scanid;		/* ID of the scan */
RID 	*NewRID;	/* the new cursor */

/* This sets the cursor of a sequential scan to a given RID.
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	the cursor (in the scan table) associated with this scan updated
  
   ERRORS:
  	e3BADSCANTYPE - if type of this scanid is not SEQUENTIAL.
  
   BUGS:
  	The given RID should be checked to be sure it is associated
  	with the same file as the scan. This is not done since
  	it would be expensive to check. Furthermore, future use of any
  	invalid cursor will be caughted by other interface routines.
*/
{
	register SCANINFO	*sptr;	/* address of scan info record */
	int	e;
	char	*dummy;

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
	{
		printf("am_setscan(scanid = %d, NewRID = ", scanid);
		PRINTRID(*NewRID); printf(")\n");
	}
#endif

	sptr = AM_getscan(scanid);	/* get address of scan info record */
	if (sptr == NULL) return(e3BADSCANID);
	if (sptr->scantype != SEQUENTIAL) return (e3BADSCANTYPE);
		
	/* here would be the place to check the validity of the new cursor */
	/*
	{ 
		e = st_readrecord(sptr->filenum, NewRID, dummy, 0,
			sptr->trans_id, FALSE, l_NL, sptr->cond);
		CHECKERROR(e);
	}
	*/

	sptr->rid = *NewRID;	/* the new cursor is set */

	return (eNOERROR);

} /* am_setscan */
