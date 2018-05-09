
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
/* Module bf_flushbuf:
    This routine flushes all the buffers of this file.
    The parameter close allows the user to also close the file 
    after flushing the buffers.  

   Imports :
    io_writepage()
    buftable[], bufpool[]

   Exports :    
    be_flushbuf(filenum, close)
  
   Errors :
    None

   Returns :
    None
*/

#include <bf.h>

bf_flushbuf(filenum, close)
register int    filenum;	/* which file */
int    	close;	 	/* if true the file should be closed also */
{
    register int i,j;	/* for loop index */
    int	e; 	/* routine return code */
    PID	spageid;


#ifdef TRACE
    if (checkset (&Trace1, tINTERFACE))
    	printf("bf_flushbuf(filenum=%d,close=%c)\n",
    		filenum,close?'T':'F');
#endif
	
    for(i = 0; i < smPtr->bf_num_bufs; i++) 
    {
	if ((smPtr->buftable[i].Bvalid) && 
		(smPtr->buftable[i].Bfilenum == filenum)) 
	{
	    /* a page belonging to the file has been found */
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
    		if (smPtr->buftable[j].Bdirty) 
    		{    
    		    (void) io_writepage(&spageid, &(smPtr->bufferpool[j]),
    				SYNCH, NULL);
    		    smPtr->buftable[j].Bdirty = FALSE;
    		}
    		if (close) 
    		{
    		    smPtr->buftable[i].Bfixcnt = 0;
    		    smPtr->buftable[i].Bfilenum = NIL;
		    smPtr->buftable[i].Bfid.Fvolid = NULL;
		    smPtr->buftable[i].Bfid.Ffilenum = NULL;
    		}
	        BF_unlock(&spageid);
	    }
	}
    }
    return(eNOERROR);
} /* end of bf_flushbuf */


