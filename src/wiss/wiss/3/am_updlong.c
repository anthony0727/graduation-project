
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
/* Module : am_updatelong
	Replace "length" bytes of text in a long data item with
	the new text in a buffer.

   IMPORTS:
  	int st_writeframe(OpenFileNum, DirRID, Offset, RecAdr, Len)
  	SCANINFO *AM_getscan(scanid)
  
   EXPORTS:
  	int	am_updatelong(ScanID, RecAdr, Length)
*/

#include	<wiss.h>
#include	<am.h>
  
extern	SCANINFO *AM_getscan();

int
am_updatelong(scanid, recaddr, length)
int	scanid;		/* ID of the scan in progress */
char	*recaddr;	/* where is the new data */
int	length;		/* # of bytes to be replaced */

/* Replace up to "length" bytes of a long data item
    starting from the cursor position of the scan
  
   RETURNS:
  	number of bytes written.
  
   SIDE EFFECTS:
  	none
  
   ERRORS:
  	e3BADSCANTYPE : if type of scanid is not LONGSCAN
*/

{
	register SCANINFO	*sptr;	/* address of scan info record */

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
		printf("am_updatelong(scanid=%d, recaddr=0x%x, length=%d)\n",
				 scanid, recaddr, length);
#endif

	sptr = AM_getscan(scanid);	/* get address of scan info */
	if (sptr == NULL) return(e3BADSCANID);

	if (sptr->scantype != LONGSCAN) return (e3BADSCANTYPE);

	if (length <= 0) return (0);

	/* replace the specified part and 
		return the actual # of bytes written */
	return(st_writeframe(sptr->filenum, &sptr->rid, sptr->offset, 
		recaddr, length, sptr->trans_id, sptr->lockup, sptr->cond));

} /* am_updatelong */
