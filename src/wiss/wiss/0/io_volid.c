
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
/* Module io_volid: Device Name to Volume ID conversions 

   IMPORTS:
	VolDev[]

   EXPORTS:
	io_volid	Convert a device name into a Volume ID
 */

#include	<wiss.h>
#include	<io.h>


io_volid(DeviceName)
register char	*DeviceName;		/* name of physical device */

/* Look for the given DeviceName in the table of on-line devices,
   and return the Volume ID of the corresponding volume.

   Returns:
	Volume ID

   Side Effects:
	NONE

   Errors:
	e0VOLNOTMOUNTED - Volume not mounted
*/
{
	register int	i;		/* for scanning table */
        int ret_val;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
		printf ("io_volid (DeviceName = \"%s\")\n", DeviceName);
#endif

	for (i = 0; i < MAXVOLS && strcmp(smPtr->VolDev[i].VDvolname,DeviceName); i++);
       {
	ret_val = (i >= MAXVOLS) ? e0VOLNOTMOUNTED : smPtr->VolDev[i].VDvolid;
	return(ret_val);
       }

} /* io_volid */

