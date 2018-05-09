
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
/* Module st_mount: mount a device 

   IMPORTS:
	io_mount(devname)
	ST_mount(volid)

   EXPORTS:
	st_mount(devname)

*/

#include	<wiss.h>
#include	<st.h>

FID     NullFID = {NULL, NULL};

st_mount (DevName)
char	*DevName;

/* mount a physical device

   Imports:
	io_mount()
	ST_mount(volid)

   Returns:
	Volume ID of mounted volume

   Side Effects :
	NONE

   Errors :
	NONE
*/
{
	register int	volid;		/* VolID of newly-mounted device */
	register int	e;		/* error code */
	int i;
#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
		printf("st_mount (DevName=%s)\n", DevName);
#endif
	SetLatch(&smPtr->level2Latch, procNum, NULL);
	volid = io_mount (DevName);	/* mount the device */
	CHECKERROR_RLATCH(volid,&smPtr->level2Latch, procNum);

	e = ST_mount(volid);	/* get FileDir info into FileTab */
	if (e < eNOERROR)
	{
	    io_dismount (DevName);
	    ReleaseLatch(&smPtr->level2Latch, procNum);
	    return(e);
	}
	else
        { 
	    ReleaseLatch(&smPtr->level2Latch, procNum);
	    return (volid);
        }
} /* st_mount */
