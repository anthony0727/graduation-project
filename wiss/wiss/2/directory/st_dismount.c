
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
/* Module st_dismount : dismount a device 

   IMPORTS:
	io_volid(devicename)
	io_dismount(devicename)
	bf_dismount(devicename)
	ST_dismount(volid)

   EXPORTS:
	st_dismount(devicename)

*/

#include	<wiss.h>
#include	<st.h>

st_dismount (DevName)
char	*DevName;

/* Dismount a named device

   Returns:
	None

   Side Effects :
	Device is removed from the system. Information in open file table 
	and file directory are updated if necessary

   Errors :
	None

*/
{
	int	e;		/* for error checking */
	int	volid;		/* ID of volume mounted on device */
	register i;

#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
		printf("st_dismount (DevName=%s)\n", DevName);
#endif
	SetLatch(&smPtr->level2Latch, procNum, NULL);
	volid = io_volid(DevName);
	CHECKERROR_RLATCH(volid,&smPtr->level2Latch, procNum);

	e = ST_dismount(volid, DevName);
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(e);

} /* st_dismount */

