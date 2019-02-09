#include <bf.h>

bf_reservepage(transId, filenum, fid, pageid, returnpage)
int	transId;
int     filenum;        /* which open file */
FID     fid;            /* which file */
PID	*pageid;	/* for which page */
PAGE	**returnpage;	/* where to return the address */
{
	register int i;
	PID	spageid;

#ifdef TRACE
	if (checkset(&Trace1, tINTERFACE))
		printf("bf_getbuf(filenum=%d,PID=%d:%d)\n",
		filenum, pageid->Pvolid, pageid->Ppage);
#endif
	/* check the input parameters */
	if (pageid == NULL) return(e1NULLPIDPARM);

	spageid = *pageid;

#ifdef EVENTS 
	BF_event(procNum, "GetBuf", &spageid, -1);
#endif 
	BF_lock(&spageid);

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
	smPtr->buftable[i].Bdirty = FALSE;

	smPtr->buftable[i].Bfixcnt = 1;
	smPtr->bf_num_free--;

	BF_insert(&spageid, i);	/* register it in the hash table */

#ifdef EVENTS 
	BF_event(procNum, "GetBuf", &spageid, i);
#endif
	*returnpage = &(smPtr->bufferpool[i]);
	BF_unlock(&spageid);
	return(eNOERROR);
}

bf_reserve_alloc_pages(tableindex_ptr, pageptr_ptr, num_wanted_pages)
int * tableindex_ptr;
char ** pageptr_ptr;
int num_wanted_pages;
{
	return(BF_allocbufs(tableindex_ptr, pageptr_ptr, num_wanted_pages));
}

bf_reserve_lock_page(transId, filenum, fid, pageid, pageptr)
int	transId;
int     filenum;        /* which open file */
FID     fid;            /* which file */
PID	*pageid;	/* for which page */
PAGE	**pageptr;	/* where to return the address */
{
	register int i;
	PID	spageid;

#ifdef TRACE
	if (checkset(&Trace1, tINTERFACE))
		printf("bf_getbuf(filenum=%d,PID=%d:%d)\n",
		filenum, pageid->Pvolid, pageid->Ppage);
#endif
	/* check the input parameters */
	if (pageid == NULL) return(e1NULLPIDPARM);

	spageid = *pageid;
	i = *pageptr - smPtr->bufferpool;

#ifdef EVENTS 
	BF_event(procNum, "GetBuf", &spageid, -1);
#endif 
	BF_lock(&spageid);

	/* BF_allocbuf leaves the allocated page fixed */
	smPtr->buftable[i].Bfilenum = filenum;
	smPtr->buftable[i].Bfid = fid;
	smPtr->buftable[i].transId = transId;
	smPtr->buftable[i].Bpageid = spageid;
	smPtr->buftable[i].Bdirty = FALSE;

	smPtr->buftable[i].Bfixcnt = 1;
	smPtr->bf_num_free--;

	BF_insert(&spageid, i);	/* register it in the hash table */

#ifdef EVENTS 
	BF_event(procNum, "GetBuf", &spageid, i);
#endif
	BF_unlock(&spageid);
	return(eNOERROR);
}