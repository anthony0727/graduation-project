
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
/* Module bf_setdirty
	This routine notifies the buffer manager that a page has 
	been modified.

	The page is known to be pinned in the buffer pool

   Imports :
	buftable[], bufferpool[]

   Exports :	
	bf_setdirty(pid)

   Returns :
	None

   Errors:
	e1NULLPIDPARM : page id missing
	e1WRONGBUFER: accessing the wrong buffer

*/

#include <bf.h>

bf_setdirty(filenum, pageid, pagebuf)
int	filenum;
PID 	*pageid;	/* which page has been modified */
PAGE	*pagebuf;
{	
	register i;
	PID	spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_setdirty(filenum=%d,page=%d:%d,pagebuf=0x%x)\n",
			filenum, pageid->Pvolid, pageid->Ppage, pagebuf);
#endif

	if (pageid == NULL)	return(e1NULLPIDPARM);
	spageid = *pageid;
	i = (int)(pagebuf - smPtr->bufferpool);
#ifdef EVENTS 
	BF_event(procNum,"SetDirty",&spageid,i);  
#endif 
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
		smPtr->buftable[i].Bdirty = TRUE;
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
