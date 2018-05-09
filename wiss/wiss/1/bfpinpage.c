#include <bf.h>

bf_pinpage(transId, filenum, fid, pageid, returnpage)
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

	*returnpage = NULL;

#ifdef EVENTS 
	BF_event(procNum, "PinBuf", &spageid, -1);
#endif 
	BF_lock(&spageid);

	/* see if the page is in the buffer pool */
	i = BF_lookup(&spageid);  /* if present (i != NIL), i is the index of the page in the buffer pool */

	if (i != NIL)
	{
		/* one more user of this buffer */
		if (smPtr->buftable[i].Bfilenum == -1)
		{
			/* adopt the file */
			smPtr->buftable[i].Bfilenum = filenum;
			smPtr->buftable[i].Bfid = fid;
			smPtr->buftable[i].transId = transId;
		}
		if (smPtr->buftable[i].Bfixcnt <= 0)
		{
			smPtr->buftable[i].Bfixcnt = 1;
			smPtr->bf_num_free--;
		}
		else smPtr->buftable[i].Bfixcnt++;

		*returnpage = &(smPtr->bufferpool[i]);
	}

#ifdef EVENTS 
	BF_event(procNum, "PinBuf", &spageid, i);
#endif

	BF_unlock(&spageid);
	return(eNOERROR);
}