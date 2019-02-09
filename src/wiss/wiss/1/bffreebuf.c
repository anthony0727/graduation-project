
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
/* Module bf_freebuf:
	This routine releases the buffer which holds the page.
	The page is known to be fixed in the buffer pool

   Imports :
	bf_num_free, buftable[], bufferpool[]

   Exports :	
	bf_freebuf(filenum, pageid, pagebuf)

   Errors :
	e1NULLPIDPARM : missing page id
	e1WRONGBUFER: accessing the wrong buffer

   Returns :
	None
*/

#include <bf.h>


bf_freebuf(filenum, pageid, pagebuf)
int	filenum;
PID	*pageid;	/* which page's buffer to free */
PAGE	*pagebuf;	/* the buffer to be released */
{
	register i;
	PID	spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_freebuf(filenum=%d,pageid=%d:%d,pagebuf=0x%x)\n", 
			filenum, pageid->Pvolid, pageid->Ppage, pagebuf);
#endif

	if (pageid == NULL) return(e1NULLPIDPARM);
	i = pagebuf - smPtr->bufferpool;

	spageid = *pageid;
#ifdef EVENTS 
	BF_event(procNum,"FreeBuf",&spageid,i);
#endif 
	BF_lock(&spageid);

	if (i >= 0 && i < smPtr->bf_num_bufs) {
/*#ifdef	DEBUG*/
		if (!PIDEQ(spageid,smPtr->buftable[i].Bpageid))
                    {
/*
			BF_unlock(&spageid);
*/
		   printf ("Couldn't find the page in the buffer pool.\n");
		   printf("bf_freebuf(filenum=%d,pageid=%d:%d,pagebuf=0x%x)\n", 
		     filenum, pageid->Pvolid, pageid->Ppage, pagebuf);
		   printf ("buffer index is %d.\n",i);
		   printf("spage is pageid=%d:%d.\n",spageid.Pvolid,spageid.Ppage);
			BF_dumpbuftable();
			BF_dumpbufpool();
/*
			BF_dumpevent ();
*/
			exit(1);
			return(e1WRONGBUFFER);
                    }
/*#endif*/
		if (--smPtr->buftable[i].Bfixcnt == 0) {
			smPtr->bf_num_free++;
			smPtr->buftable[i].Brefbit = TRUE;
		}
	}
/*#ifdef	DEBUG*/
	else 
          { 
	   	BF_unlock(&spageid);
		printf ("Couldn't find the page in the buffer pool.\n");
		printf("bf_freebuf(filenum=%d,pageid=%d:%d,pagebuf=0x%x)\n", 
		     filenum, pageid->Pvolid, pageid->Ppage, pagebuf);
		printf ("buffer index is %d.\n",i);
		printf("spage is pageid=%d:%d.\n",spageid.Pvolid,spageid.Ppage);
/*
		BF_dumpbuftable();
		BF_dumpbufpool();
		BF_dumpevent ();
*/
           	return(e1WRONGBUFFER);
          }
/*#endif*/
	BF_unlock(&spageid);
	return(eNOERROR);
}
