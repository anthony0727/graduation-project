
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
/* Module : am_closefile - Close a data file.
  
   IMPORTS:
  	int	 st_closefile(filenum)
  	int	 am_closescan(scanid)
  	int	 AM_nextscan(filenum, occurrence)
   EXPORT:
  	int	am_closefile (filenum)
*/

#include <wiss.h>
#include <am.h>

int
am_closefile (filenum)
int	 filenum;	/* open file number of the file to be closed */

/* Close the file associated with the 'filenum'.
  
   Returns: 
  	NONE
     
   Side effects: 
 	NONE
  
   Errors: 
	NONE 
*/

{
	register int 	e ;		/* to catch error codes */
	register int	scanid;		/* scan table id */
	
#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))
		printf("am_closefile(filenum = %d)\n", filenum);
#endif

	if (filenum < 0 || filenum >= MAXOPENFILES)
		return(e3BADFILENO);

	/* close leftover entries in the scan table (if any) */
	while ((scanid = AM_nextscan(filenum, NIL)) != NIL)
	{ /* remove scanid from scan table */
		e = am_closescan(scanid);
		CHECKERROR(e);
	}

	return(st_closefile(filenum, TRUE));	/* this should be done HERE! */

} /* am_closefile */

