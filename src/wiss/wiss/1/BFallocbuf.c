
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
/* Module BF_allocbuf:
	This routine returns a buffer to the caller.
	The replacement strategy is based on the CLOCK algorithm.

	Imports :
		bf_num_free, buftable[], bufferpool[]
		io_writepage(pageid, buf_address, sync, flag);

	Exports:
		int BF_allocbuf()

	Returns :
		The table index of a free buffer allocated

	Side Effects:
		A page may be swapped

	Errors :
		e1NOFREEBUFFERS : no buffer available
*/

/* Module bf_allocbuf:
	This routine allocates a buffer for a page assuming that
	the page will NOT be written on.  If a buffer already exists 
	for the page, its address is returned.

   Imports :
	bf_num_free, buftable[], bufferpool[]
	BF_allocbuf()

   Exports :	
	bf_allocbuf(pageid, returnpage)

   Errors :
	e1NULLPIDPARM : missing page id

   Returns :
	The memory address of the buffer (via returnpage)
*/

#include <bf.h>

int BF_allocbuf()
{
	register int j;	/* loop indices */
	int e;			/* for returned error code */
#ifdef TRACE
	if (checkset (&Trace1, tBUFMANAGER)) printf("BF_allocbuf()\n");
#endif
	if (smPtr->bf_num_free == 0) { /* deadlock! no replacable buffers left */
		return(e1NOFREEBUFFERS);
	}

	for (j = smPtr->clock_hand + 1;; j++)	{
		if (j == smPtr->bf_num_bufs) j = 0;
		if (!smPtr->buftable[j].Bvalid)	{
			smPtr->buftable[j].Bvalid = TRUE;
			return(smPtr->clock_hand = j); /* an unoccupied buffer */
		}
		if (smPtr->buftable[j].Bfixcnt > 0) continue;
		if (!smPtr->buftable[j].Brefbit) break;	/* a victim found */
		smPtr->buftable[j].Brefbit = FALSE; /* clear ref bit */
	}
#ifdef TRACE
	if (checkset(&Trace1, tBUFMANAGER))
		printf("buffer %d, page %3.3d:%3.3d been swapped out\n",
		j, smPtr->buftable[j].Bpageid.Pvolid, smPtr->buftable[j].Bpageid.Ppage);
#endif
	if (smPtr->buftable[j].Bdirty)	{ /* a dirty one picked, flush it out */
		e = io_writepage(&smPtr->buftable[j].Bpageid,
			&smPtr->bufferpool[j], SYNCH, NULL);
		CHECKERROR(e);
		smPtr->buftable[j].Bdirty = FALSE;
	}
	BF_delete(&smPtr->buftable[j].Bpageid);
	return(smPtr->clock_hand = j); /* return the newly allocated buffer */
} /* BF_allocbuf */

int BF_allocbufs(tableindex_ptr, pageptr_ptr, num_wanted_pages)
int * tableindex_ptr;
char ** pageptr_ptr;
int num_wanted_pages;
{
	register int j;	/* loop indices */
	int num_alloc_pages;
	int clock_start;
	int e;			/* for returned error code */
#ifdef TRACE
	if (checkset(&Trace1, tBUFMANAGER)) printf("BF_allocbufs()\n");
#endif
	if (smPtr->bf_num_free == 0) { /* deadlock! no replacable buffers left */
		return(e1NOFREEBUFFERS);
	}

	num_alloc_pages = 0;
	clock_start = smPtr->clock_hand == -1 ? 0 : smPtr->clock_hand;

	for (j = clock_start + 1;; j++)	{
		if (j == smPtr->bf_num_bufs) j = 0;

		if (!smPtr->buftable[j].Bvalid)	{
			smPtr->buftable[j].Bvalid = TRUE;
			pageptr_ptr[tableindex_ptr[num_alloc_pages]] = (char *)&(smPtr->bufferpool[j]);
			num_alloc_pages++;
			/* an unoccupied buffer */
		}
		else if (smPtr->buftable[j].Bfixcnt > 0);
		else if (!smPtr->buftable[j].Brefbit) {
			if (smPtr->buftable[j].Bdirty)	{ /* a dirty one picked, flush it out */
				e = io_writepage(&smPtr->buftable[j].Bpageid, &smPtr->bufferpool[j], SYNCH, NULL);
				CHECKERROR(e);
				smPtr->buftable[j].Bdirty = FALSE;
			}
			BF_delete(&smPtr->buftable[j].Bpageid);
			pageptr_ptr[tableindex_ptr[num_alloc_pages]] = (char *)&(smPtr->bufferpool[j]);
			num_alloc_pages++;
			/* a victim found */
		}
		else smPtr->buftable[j].Brefbit = FALSE; /* clear ref bit */

		if (j == clock_start) break;
		if (num_alloc_pages == num_wanted_pages) break;
	}
#ifdef TRACE
	if (checkset(&Trace1, tBUFMANAGER))
		printf("buffer %d, page %3.3d:%3.3d been swapped out\n",
		j, smPtr->buftable[j].Bpageid.Pvolid, smPtr->buftable[j].Bpageid.Ppage);
#endif

	smPtr->clock_hand = j;

	return num_alloc_pages; /* return the number of newly allocated buffer */
} /* BF_allocbufs */

bf_allocbuf (transId, pageid, fid, returnpage) 
int	transId;
PID	*pageid;	/* for which page */
FID	fid;
PAGE	**returnpage;	/* where to return the address */
{
	register int i;
	PID	spageid;
	int	e;


#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_allocbuf(PID=%d:%d)\n",
			pageid->Pvolid, pageid->Ppage);
#endif
	/* check the input parameters */
	if (pageid == NULL) return(e1NULLPIDPARM);

	spageid = *pageid;
	BF_lock(&spageid);  

	/* see if the page is in the buffer pool */
	i = BF_lookup(&spageid);  /* if present (i <> NIL), 
		i is the index of the page in the buffer pool */
	if (i == NIL)  /* need a new buffer */
	{	
		if ((i = BF_allocbuf()) < eNOERROR) 
		{ 
		    BF_unlock(&spageid);
		    return(i);
		}
		/* BF_allocbuf leaves the page fixed */
		smPtr->buftable[i].Bfilenum = -1;  /* don't know yet */
		smPtr->buftable[i].Bfid = fid;
		smPtr->buftable[i].transId = transId;
		smPtr->buftable[i].Bpageid = spageid;
		BF_insert(&spageid, i);	/* register it in the hash table */
		e = eNOERROR;
	}
	else 
	{ 
	    /* one more user of this buffer */
	    if (smPtr->buftable[i].Bfixcnt <=0) 
	    {
	        smPtr->buftable[i].Bfixcnt = 1;
	        smPtr->bf_num_free--;
	    }
	    else smPtr->buftable[i].Bfixcnt++;
	    
	    /* return a flag indicating page was already
	    in the buffer pool */
	    e = e1PAGEWASFOUND;
	}
	smPtr->buftable[i].Bdirty = FALSE; 

	*returnpage = &(smPtr->bufferpool[i]);
	BF_unlock(&spageid);
	return(e);
}
