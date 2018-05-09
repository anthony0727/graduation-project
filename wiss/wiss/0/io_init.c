
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
/* Module io_init: initialize level 0 tables 

   IMPORTS:
	VolDev[]

   EXPORTS:
	io_init()	Initializes level 0
*/

#include	<wiss.h>
#include	<io.h>

/*
extern LATCH disklatch;  dewitt: 9/30/90 not clear this is needed in non-gamma 
	environments but not 100% sure yet
*/

/* io_init is called only by the first process that starts up */
io_init()

/* Initialize tables for level 0

   Returns:
	NONE

   Side Effects:
	Sets VolDev to an "empty" state

   Errors:
	NONE
*/
{
	register int	i;		/* for going along VolDev */

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
		printf("io_init()\n");
#endif

	for (i = 0; i < MAXVOLS; i++)
	{
		smPtr->VolDev[i].VDvolid = NOVOL;
		smPtr->VolDev[i].VDvolname[0] = '\0';
		InitLatch(&smPtr->VolDev[i].latch);
	}
        InitLatch (&smPtr->VolDevLatch);
/*
	InitLatch (&smPtr->diskLatch);
*/
	return  (eNOERROR);

} /* io_init */


/* local_io_init is called by each process */
local_io_init()

/* Initialize tables for level 0

   Returns:
	NONE

   Side Effects:
	Sets VolDev to an "empty" state

   Errors:
	NONE
*/
{
	register int	i;		/* for going along VolDev */

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
		printf("io_init()\n");
#endif

	for (i = 0; i < MAXVOLS; i++)
	{
	    openFileDesc[i] = NOVOL;
	}
	return  (eNOERROR);

} /* local_io_init */
