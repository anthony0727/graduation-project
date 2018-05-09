
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
/* Module io_final: finalize level 0 - fulshing headers, etc.

   IMPORTS:
	VolDev[]
	IO_DisMount()

   Exports:
	io_final()	Cleans up level 0
*/

#include	<wiss.h>
#include	<io.h>


io_final()

/* Clean up level 0.

   Returns:
	NONE

   Side Effects:
	All active volume headers flushed and their device dismounted

   Errors:
	NONE
*/
{
	register int	i;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
		printf("io_final()\n");
#endif
	SetLatch(&smPtr->VolDevLatch, procNum, NULL); 
	for (i = 0; i < MAXVOLS; i++)
	{
		if (smPtr->VolDev[i].VDvolid != NOVOL) /* still mounted */
		{
			SetLatch(&smPtr->VolDev[i].latch, procNum, NULL); 
			(void) IO_DisMount(i);
			openFileDesc[i] = NOVOL;
			smPtr->VolDev[i].VDvolid = NOVOL;
			smPtr->VolDev[i].VDvolname[0] = '\0';
			ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
		}
	}
	ReleaseLatch (&smPtr->VolDevLatch, procNum);
	return (eNOERROR);

} /* io_final */

