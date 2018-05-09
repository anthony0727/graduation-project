
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
/* Module bf_findbuf:
	This routine translates a disk page id into a memory buffer address.
	If the page is not in the buffer, it is an error.  That is, this
	routine is called when the given buffer page is known to reside
	in the buffer pool.  The fix count of the buffer page is NOT
	incremented by this routine.

   Imports :
	buftable[], bufferpool[]

   Exports :	
	bf_findbuf(filenum, pageid, returnpage)

   Returns :
	the memory address of the buffer

   Errors:
	e1NULLPIDPARM   : page id missing

*/

#include <bf.h>


bf_findbuf(transId, filenum, fid, pageid, returnpage)
int	transId;
int	filenum;	/* which open file */
FID	fid;		/* which file */
PID	*pageid;	/* which page */
PAGE	**returnpage;	/* where to return the address */
{
	register	i;
	PID	spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_findbuf(filenum=%d,page=%d:%d)\n",
			filenum, pageid->Pvolid, pageid->Ppage);
#endif
	/* check the input parameters */
	if (pageid == NULL) return(e1NULLPIDPARM);

	spageid = *pageid;

#ifdef EVENTS 
	BF_event(procNum,"FindBuf",&spageid,0);   
#endif 

	BF_lock(&spageid);

	if ( (i = BF_lookup(&spageid)) == NIL) 
	{
		BF_unlock(&spageid);
		return(e1WRONGBUFFER);
	}
	else 
	{
		if (smPtr->buftable[i].Bfilenum == -1) 
		{
			smPtr->buftable[i].Bfilenum = filenum;	
				/* adopt the file */
			smPtr->buftable[i].Bfid = fid;
			smPtr->buftable[i].transId = transId;
		}
	}
	*returnpage = &(smPtr->bufferpool[i]);
	BF_unlock(&spageid);

	return(eNOERROR);
}  /* end bf_findbuf */

