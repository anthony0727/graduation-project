
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
/* Module st_devsync : make the contents of a storage device update-to-date

   IMPORTS:
	io_volid(devicename)
	bf_dismount(devicename)

   EXPORTS:
	st_devsync(devicename)

*/

#include	<wiss.h>
#include	<st.h>

st_devsync (DevName)
char	*DevName;

/* Fulsh all the disk pages of a device in memory.

   Returns:
	None

   Side Effects :
	Make the memory state of a device "clean"

   Errors :
	None

*/
{
	int	e;		/* for error checking */
	register i;
	int	volid;		/* ID of volume mounted on device */

#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
		printf("st_devsync (DevName=%s)\n", DevName);
#endif
	SetLatch(&smPtr->level2Latch, procNum, NULL);

	volid = io_volid(DevName);
	CHECKERROR_RLATCH(e,&smPtr->level2Latch, procNum);

	/* flush each file reside on the device */
	for (i = 0; i < MAXOPENFILES; i++)
		if (smPtr->files[i].ptr != NIL && F_VOLUMEID(i) == volid) 
			(void) bf_flushbuf(i, FALSE);

	/* flush the file directory header,
           currently do nothing because the header is always "write through" 
	*/

	/* flush the header of the volume */
	
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(eNOERROR);

} /* st_devsync */

