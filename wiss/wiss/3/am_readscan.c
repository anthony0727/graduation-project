
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
/* Module : am_readscan
  	Reads the current record of the scan into 'recaddr'.

   IMPORTS:
  	int	st_readrecord(openfileno, ridptr, recaddr, len, trans_id, 
		lockup, mode, cond)
	SCANINFO *AM_getscan();
  
   EXPORTS:
  	int	am_readscan(scanid, recaddr, len)
*/

#include	<wiss.h>
#include	<am.h>
#include 	<lockquiz.h>
  
extern	SCANINFO *AM_getscan();

int
am_readscan(scanid, recaddr, len)
int	  scanid;	/* ID of the scan in progress */
char  	  *recaddr;	/* where to return the data read */
int	  len;		/* length of the record */

/* Reads the current record of the scan into 'recaddr'.
  
   RETURNS:
  	number of bytes actually read
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	e3BADSCANTYPE - if scan type = LONG
*/
{
	register SCANINFO	*sptr;		/* address of the scan info record */

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
		printf("am_readscan(scanid=%d, recaddr=0x%x, length=%d)\n",
				scanid, recaddr, len);

#endif

	sptr = AM_getscan(scanid);	/* get address of scan info record */
	if (sptr == NULL) return(e3BADSCANID);
	if (sptr->scantype == LONGSCAN) return (e3BADSCANTYPE);

	return(st_readrecord(sptr->filenum, &sptr->rid, recaddr, len, 
		sptr->trans_id, sptr->lockup, sptr->mode, sptr->cond));

}  /* am_readscan */
