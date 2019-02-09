
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
/* Module bf_invalidate:
	This routine invalidates a set of buffers.

   Imports :
	buftable[], bufpool[]

   Exports :	
	bf_invalidate(num_pages, pids)
  
   Errors :
	None

   Returns :
	None
*/

#include <bf.h>

bf_invalidate(num_pages, pids)
register int	num_pages;	/* how many */
PID		*pids;
{
	register int i, j;	/* for loop index */
	int	ix;
	PID	pid, spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_invalidate(num_pages=%d,pids=0x%x)\n",
			num_pages, pids);
#endif
    for(i = 0; i < smPtr->bf_num_bufs; i++) 
    {
	if (smPtr->buftable[i].Bvalid) 
	{
	    spageid = smPtr->buftable[i].Bpageid; /* A */
	    BF_lock(&spageid);	/* B */

	    /* 
	      now look up the page to gets its frame.  yes,
	      ix should equal i after the call below but
	      another process may have deleted the page from
	      the buffer pool between statements A and B above 
	    */

	    ix = BF_lookup(&spageid);
	    if (ix == NIL) /* page is no longer in the buffer pool */
	    {
		BF_unlock(&spageid);
	    }
	    else
	    {
		for (j = 0; j < num_pages; j++) 
		{
		    pid = pids[j];
		    if (PIDEQ(spageid, pid)) 
		    {
			smPtr->buftable[ix].Bvalid = FALSE;
			BF_delete(&pid);
			break;
		    }
		}
		BF_unlock(&spageid);
	    }
	}
    }
    return(eNOERROR);

} /* end of bf_invalidate */

