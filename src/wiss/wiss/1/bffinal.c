
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
/* Module bf_final:
	This routine finalizes (clean up) all the level 1 data structures.

   Imports :
	bf_num_free, buftable[], bufferpool[]
	BF_hashinit();

   Exports :	
	bf_final()

   Errors :
	None

   Returns :
	None
*/

#include <bf.h>

bf_final()
{
	register int i;
	PID	spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_final()\n");
#endif
	for(i = 0; i < smPtr->bf_num_bufs; i++) 
	{
		if (smPtr->buftable[i].Bvalid && smPtr->buftable[i].Bdirty) 
		{
			spageid = smPtr->buftable[i].Bpageid;
			BF_lock(&spageid);
			(void)io_writepage (&spageid, &(smPtr->bufferpool[i]), SYNCH,
			    NULL);
			BF_unlock(&spageid);
		}
		
		smPtr->buftable[i].Bvalid = FALSE;/* mark the buffer free */
		smPtr->buftable[i].Bdirty = FALSE;
		smPtr->buftable[i].busy = FALSE;
	}
	smPtr->bf_num_free = smPtr->bf_num_bufs;
/*
deleted by dewitt while doing the system V port. should be ok
	BF_hashinit();	
*/

	/* dump the event table */
	/* BF_dumpevent();  */

	return(eNOERROR);
}
