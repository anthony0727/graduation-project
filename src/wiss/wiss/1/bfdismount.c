
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
/* Module bf_dismount:
    This routine releases all the buffers that contains a page on the volume. 
    If a page to be released is dirty, it is flushed to disk first.

   Imports :
    buftable[]
    io_writepage(pageid, bufaddress, SYNCH, flag)

   Exports :	
    bf_dismount(volid)

   Returns :
    None
   Bugs:
    locks all of the buffer pool while dismounting a volume
    
*/

#include <bf.h>


bf_dismount(volid)
int	volid;
{
    register int i, j;
    int		e;
    PID	spageid;

#ifdef TRACE
    if (checkset (&Trace1, tINTERFACE))
 		printf("bf_dismount(volid=%d)\n", volid);
#endif
#ifdef EVENTS
    BF_dumpevent(); 
#endif

    /* release all the buffers which hold pages of this volume */
    for(i = 0; i < smPtr->bf_num_bufs; i++) 
    {
	if ((smPtr->buftable[i].Bvalid) && 
	   (smPtr->buftable[i].Bpageid.Pvolid == volid))
	{
	    /* a page belonging to the volume has been found */
	    spageid = smPtr->buftable[i].Bpageid; /* A */
	    BF_lock(&spageid);			  /* B */

	    /* 
	      now look up the page to gets its frame.  yes,
	      j should equal i after the call below but
	      another process may have deleted the page from
	      the buffer pool between statements A and B above 
	    */
	    j = BF_lookup(&spageid);
	    if (j == NIL) /* page is no longer in the buffer pool */
	    {
		BF_unlock(&spageid);
	    }
	    else
	    {
		/* the page was still in the buffer pool */
		/* if the page is dirty flush it to disk */
    		if (smPtr->buftable[j].Bdirty) 
    		{ 
    		    e=io_writepage(&spageid, &(smPtr->bufferpool[j]),
				SYNCH,NULL);
    		    CHECKERROR(e);
    		    smPtr->buftable[j].Bdirty = FALSE;
		}
    		BF_delete(&spageid);
    		smPtr->buftable[j].Bvalid = FALSE; /* mark the buffer free */
    		smPtr->buftable[j].Bfixcnt = 0; 
    		BF_unlock(&spageid);
	    }
	}
    }
    return(eNOERROR);
} 

