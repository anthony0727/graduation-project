
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
/* Module bf_closefile:
	This routine assumes that any writing of this users buffers for this
	file has already been done by the user.  
	It closes a file by releasing all the buffers for this file.

   Imports :
	buftable[]

   Exports :	
	bf_closefile(filenum)

   Errors :
	None

   Returns :
	None
*/

#include <bf.h>

bf_closefile (filenum)
int	filenum;
{
    register	int i, j;
    PID		spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_closefile(filenum=%d)\n", filenum);
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
	        smPtr->buftable[j].Bfilenum = NIL;	/* an orphant */
	        /* in case someone left the page fixed, unfix it */
	        if (smPtr->buftable[j].Bfixcnt > 0)
		{
	            smPtr->buftable[j].Bfixcnt = 0;
		    smPtr->bf_num_free++;
		    smPtr->buftable[j].Brefbit = TRUE;
	        }
	        smPtr->buftable[j].Bfid.Fvolid = NULL;
	        smPtr->buftable[j].Bfid.Ffilenum = NULL;
	        BF_unlock(&spageid);
	    }
	}
    }
    return(eNOERROR);

} /* bfclosefile */

