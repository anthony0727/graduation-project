
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
/* Module : am_openhash
  	open a hash file
  
   IMPORTS:
  	int	st_openhash()
  
   EXPORTS:
  	int	am_openhash(VolID, FileName, HashNo, AccessMode)
*/

#include <wiss.h>	
#include <am.h>
  
int
am_openhash (volid, filename, Hashno, accessmode)
int	volid; 		/* volume id of the file */
char 	*filename;	/* a pointer to the filename */
int	Hashno;
int	accessmode;	/* indicates readonly or read/write */

/* Open a hash file.
  
     Returns:
  	open file number
  
     Side Effects:
  	None
  
     Errors generated here:
  	None
*/
{

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))  
		printf("am_openhash(volid=%d,filename=%s,hno=%d,mode=%d)\n",
				volid, filename, Hashno, accessmode);
#endif

	return(st_openhash(volid, filename, Hashno, accessmode));

} /* am_openhash */
