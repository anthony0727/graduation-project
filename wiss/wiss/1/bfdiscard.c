
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
/* Module bf_discard:
	This routine invalidates the buffer holding the specified page.
	The page is known to be fixed in the buffer pool

   Imports :
	bf_num_free, buftable[], bufferpool[]

   Exports :	
	bf_discard(filenum, pageid, pagebuf)

   Errors :
	e1NULLPIDPARM : missing page id
	e1WRONGBUFER: accessing the wrong buffer

   Returns :
	None
*/

#include <bf.h>

bf_discard(filenum, pageid, pagebuf)
int	filenum;	/* its file number */
PID	*pageid;	/* which page's buffer to free */
PAGE	*pagebuf;	/* the buffer associated with the page */
{
	register i;
	PID	spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_discard(filenum=%d, pageid=%d:%d, pagebuf=0x%x)\n", 
			filenum, pageid->Pvolid, pageid->Ppage, pagebuf);
#endif

	if (pageid == NULL) 
		return(e1NULLPIDPARM);

	i = pagebuf - smPtr->bufferpool;
	spageid = *pageid;
	BF_lock(&spageid);

	if (i >= 0 && i < smPtr->bf_num_bufs) 
	{

#ifdef	DEBUG
		if (!PIDEQ(spageid, smPtr->buftable[i].Bpageid))
                      { 
			BF_unlock(&spageid);
			return(e1WRONGBUFFER);
                      }
#endif

		/* the following will not deadlock as the fixcnt
		for the page is guaranteed to be greater than 1 */
		SetLatch(&smPtr->buftable[i].latch, procNum, NULL);
		if (--smPtr->buftable[i].Bfixcnt == 0) {
			smPtr->bf_num_free++;
			smPtr->buftable[i].Bvalid = FALSE;
			smPtr->buftable[i].Bdirty = FALSE; 
			BF_delete(&spageid);
		}
		ReleaseLatch(&smPtr->buftable[i].latch, procNum);
	}
#ifdef	DEBUG
	else
             { 
		BF_unlock(&spageid);
               	return(e1WRONGBUFFER);
             }
#endif

        BF_unlock(&spageid);
	return(eNOERROR);

}
