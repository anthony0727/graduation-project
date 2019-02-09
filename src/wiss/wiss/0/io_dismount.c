
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
/* Module io_dismount: dismount devices 

   IMPORTS:
	VolDev[]
	IO_DisMount()

   EXPORTS:
	io_dismount()	dismount a active device
 */

#include	<wiss.h>
#include	<io.h>


io_dismount(DeviceName)
char	*DeviceName;			/* physical name of device */
/* Dismount a named device

   Returns:
	NONE

   Side Effects:
	volume header flushed if necessary; 
	the device information removed from VolDev

   Errors generated here:
	e0VOLNOTMOUNTED - volume not mounted
*/
{
	register i;	/* volume's index in VolDev */
	int	e;	/* for error returns */

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
		printf("io_dismount(DeviceName=\"%s\")\n", DeviceName);
#endif

	SetLatch(&smPtr->VolDevLatch, procNum, NULL); 

	/* search in the volume/device table for the named device */
	for (i = 0; strcmp(DeviceName,smPtr->VolDev[i].VDvolname); )
		if (++i >= MAXVOLS)
                { 
			ReleaseLatch (&smPtr->VolDevLatch, procNum);
			return (e0VOLNOTMOUNTED); /* device is not mounted! */
                 }

	SetLatch(&smPtr->VolDev[i].latch, procNum, NULL); 
	/* flush volume header and dismount device */
	e = IO_DisMount(i);	
	if (e < eNOERROR) 
	{
		ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
		ReleaseLatch (&smPtr->VolDevLatch, procNum);
		return (e);
	}

	/* invalidate the table entry */
	smPtr->VolDev[i].VDvolname[0] = '\0';
	smPtr->VolDev[i].VDvolid = NOVOL;
	openFileDesc[i] = NOVOL;
	ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
	ReleaseLatch (&smPtr->VolDevLatch, procNum);
	return (eNOERROR);

} /* io_dismount */
