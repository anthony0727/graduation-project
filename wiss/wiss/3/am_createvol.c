
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
/* Module : am_create_volume
  	create a wiss volume
  
   IMPORTS:
  	int	createVolume()
  
   EXPORTS:
  	int	am_create_volume(DevName, Title, VolID, numExts, extSize)
*/

#include <wiss.h>	
  
int
am_create_volume (DevName, Title, VolID, numExts, extSize)
char 	*DevName;	/* full unix path where volume should be created */
char 	*Title;		/* title for the volume */
int	VolID; 		/* volume id of the file */
int	numExts; 	/* number of extents in the volume */
int	extSize; 	/* number of pages in each extent */

/* creates a wiss volume and formats it 
  
     Returns:
  	error code
  
     Side Effects:
  	None
  
     Errors generated here:
  	None
*/
{
	return(IO_Format(DevName, Title, VolID, numExts, extSize));

} 
