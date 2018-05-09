
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
/* Module : am_setcursor
  	Set the cursor of a long data item scan.
  	
   IMPORTS:
  	SCANINFO *AM_getscan(ScanID)
  
   EXPORTS:
  	int	am_setcursor(ScanID, Offset, Relocation)
*/
  
#include	<wiss.h>
#include	<am.h>
#include	<st_r.h>
#include	<lockquiz.h>

extern	SCANINFO *AM_getscan();

int
am_setcursor(scanid, offset, relocation)
int	scanid;		/* ID of the scan in progress */
int	offset;		/* new location of the cursor */
short	relocation;	/* 0, 1 or 2; see description above */

/* Set cursor of a long data item scan to the specified "offset" 
  	according to "relocation".
  
  	Values for "relocation" are:
  	0 : position relative to the first byte of the long item.
  	1 : position relative to the current cursor position.
  	2 : position relative to the last byte of the long item (negative offset).
  
   RETURNS:
  	eNOERROR if successful
  
   SIDE EFFECTS:
  	none
  
   ERRORS:
  	e3BADRELOCATION : if incorrect relocation value is used
  	e3BADSCANTYPE : if type associated with scanid is not LONGSCAN.
*/
{
	register SCANINFO *sptr;	/* address of scan info record */

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
		printf("am_setcursor(scanid=%d,offset=%d,relocation=%d)\n",
			scanid, offset, relocation);
#endif

	sptr = AM_getscan(scanid);	/* get address of scan info record */
	if (sptr == NULL) return(e3BADSCANID);

	if (sptr->scantype != LONGSCAN)
		return (e3BADSCANTYPE);		/* this routine is for long */

	switch (relocation)
	{
		case 0:	/* relative to the first byte in long data item */
			sptr->offset = offset;
			break;

		case 1:	/* relative to current offset position */
			sptr->offset += offset;
			break;

		case 2:	/* relative to last byte in long data item */
			/*
			sptr->offset -= offset + 1;
			break;
			*/
			/*
			** CHOU made the following correction to this 
			** code.  The above two commented lines are the 
			** way the code use to be before.  CHOU suggested 
			** that we change the above two lines by the 
			** following
			*/ 
			{
			    char   buf[sizeof(LONGDIR)];
			    int    e;
			    /* read in the directory of the long data item */
			    e = st_readrecord(sptr->filenum, &sptr->rid, 
				buf, sizeof(buf), sptr->trans_id, sptr->lockup,
				l_S, sptr->cond);
			    CHECKERROR(e);
			    sptr->offset = ((LONGDIR *)buf)->total_length;
			}
			if (offset) sptr->offset -= offset + 1;
			break;

		default: /* invalid relocation operation */
			return (e3BADRELOCATION);
	}

	return(eNOERROR);

} /* am_setcursor */

