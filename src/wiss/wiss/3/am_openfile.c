
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
/* Module : am_openfile
  	open a data file
  
   IMPORTS:
  	int	st_openfile()
  
   EXPORTS:
  	int	am_openfile(VolID, FileName, IndexNo)
*/

#include <wiss.h>	
#include <am.h>
  
int
am_openfile (volid, filename, accessmode)
int	volid; 		/* volume id of the file */
char 	*filename;	/* a pointer to the filename */
int	accessmode;	/* indicates readonly or read/write */

/* Open a file.
  
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
		printf("am_openfile(volid=%d,filename=%s,accessmode=%d)\n",
				volid, filename, accessmode);
#endif

	return(st_openfile(volid, filename, accessmode));

} /* am_openfile */
