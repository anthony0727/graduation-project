
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
/* Module : am_openindex
  	open an index
  
   IMPORTS:
  	int	st_openbtree()

   EXPORTS:
  	int	am_openindex(VolID, FileName, IndexNo, AccessMode)
*/

#include <wiss.h>	
#include <am.h>
  
int
am_openindex (volid, filename, indexno, accessmode)
int	volid; 		/* volume id of the file */
char 	*filename;	/* a pointer to the filename */
int	indexno;	/* index id */
int	accessmode;	/* indicates readonly or read/write */

/* open an index
  
     Returns:
  	open file number
  
     Side Effects:
  	NONE
  
     Errors:
  	none 
*/
{

#ifdef TRACE
	if (checkset (&Trace3, tINTERFACE))  
	{
		printf("am_openindex(volid=%d, filename=%s,", volid, filename);
		printf(" indexno=%d, accessmode=%d)\n", indexno, accessmode);
	}
#endif

	return(st_openbtree(volid, filename, indexno, accessmode));

} /* am_openindex */
