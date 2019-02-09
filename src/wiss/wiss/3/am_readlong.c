
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
/* Module : am_readlong
  	Reads up to "length" bytes of data from a long data item
  	into a given buffer.

   IMPORTS:
  	int	st_readframe(OpenFileNum, *DirRID, Offset, RecAdr, Len)
  	SCANINFO *AM_getscan(scanid)
  
   EXPORTS:
  	int	am_readlong(ScanID, RecAdr, Length)
*/

#include	<wiss.h>
#include	<am.h>
  
extern	SCANINFO *AM_getscan();

int
am_readlong(scanid, recaddr, length)
int	scanid;		/* ID of the scan in progress */
char	*recaddr;	/* where to return the data read */
int	length;		/* # of bytes to be read */

/* Reads up to "length" bytes of a long data item
  	into "recaddr" from the cursor position of the scan.
  
   RETURNS:
  	number of bytes read.
  
   SIDE EFFECTS:
  	none
  
   ERRORS:
  	e3BADSCANTYPE : if type of scanid is not LONGSCAN
*/

{
	register SCANINFO *sptr;	/* address of scan info record */

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
		printf("am_readlong(scanid=%d, recaddr=0x%x, length=%d)\n",
				 scanid, recaddr, length);
#endif

	sptr = AM_getscan(scanid);	/* get address of scan info */
	if (sptr == NULL) return(e3BADSCANID);

	if (sptr->scantype != LONGSCAN) return (e3BADSCANTYPE);

	if (length <= 0) return (0);

	/* read the specified part and return the actual # of bytes read */
	return(st_readframe(sptr->filenum, &sptr->rid, 
			sptr->offset, recaddr, length, sptr->trans_id,
			sptr->lockup, sptr->cond));

} /* am_readlong */
