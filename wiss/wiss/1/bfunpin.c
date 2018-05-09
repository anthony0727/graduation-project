
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
/* Module bf_unpin:
	This routine releases the buffer which holds the page
	and marks the page dirty if it has been dirtied.
	Page is known to be pinned 

   Imports :
	bf_num_free, buftable[], bufferpool[]

   Exports :	
	bf_unpin(pagebuf, dirty)

   Returns :
	None
*/

#include <bf.h>


bf_unpin(pagebuf, dirty)
PAGE	*pagebuf;	/* the buffer to be unpinned */
int	dirty;
{
	register i;
	PID	spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_unpin(pagebuf=0x%x, dirty=%d)\n", 
			pagebuf, dirty);
#endif
	i = pagebuf - smPtr->bufferpool;
	spageid = smPtr->buftable[i].Bpageid;
	BF_lock(&spageid);

	if (i >= 0 && i < smPtr->bf_num_bufs) 
	{
	    if (smPtr->buftable[i].Bfixcnt == 1)
	    {
		smPtr->bf_num_free++;
		smPtr->buftable[i].Brefbit = TRUE;
		if (dirty == TRUE) smPtr->buftable[i].Bdirty = TRUE;
	        smPtr->buftable[i].Bfixcnt = 0;
	    }
	    else smPtr->buftable[i].Bfixcnt--;
	}
	else 
        { 
	    BF_unlock(&spageid);
	    printf ("Couldn't find the page in the buffer pool.\n");
	    printf("bf_unpin(pagebuf=0x%x)\n", pagebuf);
	    printf ("buffer index is %d.\n",i);
/*
	    BF_dumpbuftable();
	    BF_dumpbufpool();
	    BF_dumpevent ();
*/
            return(e1WRONGBUFFER);
        }
	BF_unlock(&spageid);
	return(eNOERROR);
}
