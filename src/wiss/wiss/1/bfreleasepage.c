#include <bf.h>


bf_releasepage(filenum, pageid, pagebuf)
PID	*pageid;	/* which page's buffer to free */
PAGE	*pagebuf;	/* the buffer to be released */
{
	register i;
	PID	spageid;

#ifdef TRACE
	if (checkset(&Trace1, tINTERFACE))
		printf("bf_freebuf(filenum=%d,pageid=%d:%d,pagebuf=0x%x)\n",
		filenum, pageid->Pvolid, pageid->Ppage, pagebuf);
#endif

	if (pageid == NULL) return(e1NULLPIDPARM);
	i = pagebuf - smPtr->bufferpool;

	spageid = *pageid;
#ifdef EVENTS 
	BF_event(procNum, "FreeBuf", &spageid, i);
#endif 
	BF_lock(&spageid);

	if (--smPtr->buftable[i].Bfixcnt == 0) {
		smPtr->bf_num_free++;
		smPtr->buftable[i].Brefbit = TRUE;
	}

	BF_unlock(&spageid);
	return(eNOERROR);
}
