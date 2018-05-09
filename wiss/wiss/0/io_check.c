
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
/* Module io_check : volume header consistency checker 

   IMPORTS:
	VolDev[]
	IO_checker()

   EXPORTS:
	io_checker()	level 0 volume header checker
 */

#include	<wiss.h>
#include	<io.h>


io_checker(VolID)
register VolID;	/* volume ID */

/* This routine checks the consistency of a volume header

   Returns:
	None

   Side effects:
	Error messages, if any, go to the standard output.

   Errors:
	e0VOLNOTMOUNTED - volume not mounted
*/
{
	register i;
        int ret_val;

#ifdef	TRACE
	if ( checkset(&Trace0, tINTERFACE) )
		printf("io_checker (VolID=%d)\n", VolID);
#endif

	/* look up the volume */
	for (i = 0; smPtr->VolDev[i].VDvolid != VolID; )
		if (++i >= MAXVOLS) return(e0VOLNOTMOUNTED);
	SetLatch(&smPtr->VolDev[i].latch, procNum, NULL);
	ret_val = IO_checker(i);
	ReleaseLatch(&smPtr->VolDev[i].latch, procNum);
	return(ret_val);
	
} /* io_checker */

