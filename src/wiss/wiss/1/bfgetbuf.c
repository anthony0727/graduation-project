
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
/* Module bf_getbuf:
	This routine allocates a buffer for the page of a file, 
	presuming that the page WILL BE written on.
	If a buffer already exists for the page, its address is returned.

   Imports :
	bf_num_free, buftable[], bufferpool[]
	BF_allocbuf()

   Exports :	
	bf_getbuf(pageid, filenum, returnpage)

   Errors :
	e1NULLPIDPARM : missing page id

   Returns :
	The memory address of the buffer (via returnpage)
*/

#include <bf.h>

bf_getbuf (transId, filenum, fid, pageid, returnpage) 
int	transId;
int     filenum;        /* which open file */
FID     fid;            /* which file */
PID	*pageid;	/* for which page */
PAGE	**returnpage;	/* where to return the address */
{
	register int i;
	PID	spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_getbuf(filenum=%d,PID=%d:%d)\n",
			filenum, pageid->Pvolid, pageid->Ppage);
#endif
	/* check the input parameters */
	if (pageid == NULL) return(e1NULLPIDPARM);

	spageid = *pageid;

#ifdef EVENTS 
	BF_event(procNum,"GetBuf",&spageid,-1);  
#endif 
	BF_lock(&spageid);  

	/* see if the page is in the buffer pool */
	i = BF_lookup(&spageid);  /* if present (i <> NIL), i is the index
		of the page in the buffer pool */
	if (i == NIL)  /* need a new buffer */
	{	
		if ((i = BF_allocbuf()) < eNOERROR) 
		{ 
		    BF_unlock(&spageid);
		    return(i);
		}
		/* BF_allocbuf leaves the allocated page fixed */
		smPtr->buftable[i].Bfilenum = filenum;
		smPtr->buftable[i].Bfid = fid;
		smPtr->buftable[i].transId = transId;
		smPtr->buftable[i].Bpageid = spageid;

		smPtr->buftable[i].Bfixcnt = 1;
		smPtr->bf_num_free--;

		BF_insert(&spageid, i);	/* register it in the hash table */
	}
	else 
	{ 	
		/* one more user of this buffer */
		if (smPtr->buftable[i].Bfilenum == -1) 
		{
			/* adopt the file */
			smPtr->buftable[i].Bfilenum = filenum;	
			smPtr->buftable[i].Bfid = fid;
			smPtr->buftable[i].transId = transId;
		}
	        if (smPtr->buftable[i].Bfixcnt <=0) 
	        {
	            smPtr->buftable[i].Bfixcnt = 1;
	            smPtr->bf_num_free--;
	        }
	        else smPtr->buftable[i].Bfixcnt++;
	}
	smPtr->buftable[i].Bdirty = TRUE; 

#ifdef EVENTS 
	BF_event(procNum,"GetBuf",&spageid,i);  
#endif
	*returnpage = &(smPtr->bufferpool[i]);
	BF_unlock(&spageid);
	return(eNOERROR);
}
