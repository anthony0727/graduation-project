
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
/* Module : am_deletelong
  	Deletes a given length of data from a long data item.
  
   IMPORTS:
  	int	AM_nextscan(OpenFileNum, CurrentScanID)
  	SCANINFO *AM_getscan(scanid)
  	int	st_deleteframe(OpenFileNum, DirRID, Offset, Length)
  
   EXPORTS:
  	int		am_deletelong(scanid);
*/
  
#include	<wiss.h>
#include	<am.h> 

extern	SCANINFO *AM_getscan();

int
am_deletelong(scanid, length)
int	 scanid;	/* ID of the scan in progress */
int	 length;	/* # of bytes to be deleted */

/* Deletes a given length of data from a long data item.
  
   RETURNS:
  	total number of bytes deleted.
  
   SIDE EFFECTS:
  	the cursors of OTHER scans on this long data item may be adjusted.
  
   ERRORS:
  	e3BADSCANTYPE - if scan type is not LONGSCAN
*/
{
	register SCANINFO	*sptr;	/* address of scan info record */
	register int	delete_count;	/* # of bytes actually deleted */
	SCANINFO	*sptr1;		/* address of scan info record */

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE)) 
		printf("am_deletelong(scanid=%d,length=%d)\n", scanid, length);
#endif

	sptr = AM_getscan(scanid);	/* get address of scan info record */
	if (sptr == NULL) return(e3BADSCANID);

	if (sptr->scantype != LONGSCAN)
		return (e3BADSCANTYPE);	/* only long scan is allowed here */

	if (length <= 0) return(0);

	/* delete bytes from the long data item */
	delete_count = st_deleteframe(sptr->filenum, &sptr->rid, sptr->offset, 
	   length, sptr->trans_id, sptr->lockup, sptr->cond);
	CHECKERROR(delete_count);

	/* adjust the cursors of other scans on the same long data item 
	   if necessary (ie, all cursors after the cursor of the current 
	   scan will be shifted backward)                               */

	for (scanid = AM_nextscan(sptr->filenum, NIL); /* first scan on chain */
		scanid != NIL; scanid = AM_nextscan(sptr->filenum, scanid))
	{
		sptr1 = AM_getscan(scanid);
		if (sptr1 == NULL) return(e3BADSCANID);
		if (sptr1->scantype != LONGSCAN || !RIDEQ(sptr->rid,sptr1->rid))
			continue;	/* not a scan on this item */
		if (sptr1->offset > sptr->offset)
			continue;	/* cursor not affected */
		if ((sptr1->offset -= delete_count) < sptr->offset)
			sptr1->offset = sptr->offset;	/* adjust cursor */
	}

	return (delete_count);	/* return # of bytes deleted */

} /* am_deletelong */
