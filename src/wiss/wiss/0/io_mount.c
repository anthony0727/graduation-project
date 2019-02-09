
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
/* Module io_mount: mount a named device 

   IMPORTS:
	VolDev[]
	IO_Mount()

   EXPORTS:
	io_mount(DeviceName)	Mounts a device
 */

#include	<wiss.h>
#include	<io.h>

extern	char *strcpy();

io_mount(DeviceName)
char	*DeviceName;			/* name of device to mount */
/* Mount a named device

   Returns:
	ID of the mounted volume

   Side Effects:
	Bring the device on line and Cache device information in VolDev

   Errors generated here:
	e0TOOMANYVOLS : too many volumes mounted
*/
{
	register int	i;		/* index into VolDev of info */
	int		e;		/* for error returns */
        int ret_val;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
		printf("io_mount(DeviceName=\"%s\")\n", DeviceName);
#endif
     
	SetLatch (&smPtr->VolDevLatch, procNum, NULL);
	/* see if it's already mounted ? */
	for (i = 0; i < MAXVOLS; i++)
	    if (!strcmp(smPtr->VolDev[i].VDvolname, DeviceName))
            { 
		
		/* volume is already mounted, perhaps by another process */
		/* if this process does not have the device open do so */
		if (openFileDesc[i] == NOVOL) 
		{
		    IO_Open(DeviceName, i); /* open the device */
		}
		ReleaseLatch (&smPtr->VolDevLatch, procNum);
		return(smPtr->VolDev[i].VDvolid);
            }

	/* look for an empty entry and mount the device */
	for (i = 0; smPtr->VolDev[i].VDvolid != NOVOL; )
	    if (++i >= MAXVOLS)
	    { 
			ReleaseLatch (&smPtr->VolDevLatch, procNum);
			return(e0TOOMANYVOLS);	/* table full ! */
	    }
	SetLatch(&smPtr->VolDev[i].latch, procNum, NULL); 
	e = IO_Mount(DeviceName, i);
	if (e < eNOERROR)
	{
		ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
		ReleaseLatch (&smPtr->VolDevLatch, procNum);
		return(e);
	}

	/* validate the table entry by initializing logical volume info */
	(void) strcpy(smPtr->VolDev[i].VDvolname, DeviceName);
	ret_val = smPtr->VolDev[i].VDvolid = 
		smPtr->VolDev[i].VDheader[MHEADER]->VH.VHvolid;

	ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
	ReleaseLatch (&smPtr->VolDevLatch, procNum);
	return(ret_val);

} /* io_mount */
