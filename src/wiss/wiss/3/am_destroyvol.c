
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
/* Module : am_destroy_volume
  	destroy a wiss volume
  
   IMPORTS:
  	int	destroyVolume()
  
   EXPORTS:
  	int	am_destroy_volume(DevName)
*/

#include <wiss.h>	
#include <io_error.h>
  
int
am_destroy_volume (DevName)
char 	*DevName;	/* full unix path of volume */

/* destroy a wiss volume 
  
     Returns:
  	error code
  
     Side Effects:
  	None
  
     Errors generated here:
  	None
*/
{
	int i, result;

     	/* first see if the volume is mounted and if so return an error */

     	i = io_volid(DevName);
     	if (i >= 0) result = e0VOLMOUNTED;
	else result = unlink (DevName);
	return(result);
} 
