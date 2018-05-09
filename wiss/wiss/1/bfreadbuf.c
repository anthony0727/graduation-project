
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
/* Module bf_readbuf:
	This routine translates a disk page id into a memory buffer address.
	If the page is not in the buffer, a buffer is allocated and the disk
	page is read into that buffer.

   Imports :
	bf_num_free, buftable[], bufferpool[]
	BF_allocbuf()
	io_readpage(pageid, bufaddress) 

   Exports :	
	bf_readbuf(filenum, pageid, returnpage)

   Returns :
	the memory address of the buffer

   Errors:
	e1NULLPIDPARM   : page id missing

*/

#include <bf.h>

bf_readbuf(transId, filenum, fid, pageid, returnpage)
int	transId;
int     filenum;        /* which open file */
FID     fid;            /* which file */
PID	*pageid;	/* which page */
PAGE	**returnpage;	/* where to return the address */
{
	register	i;
	int	e;
	PID	spageid;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_readbuf(filenum=%d,page=%d:%d)\n",
			filenum, pageid->Pvolid, pageid->Ppage);
#endif

	/* check the input parameters */
	if (pageid == NULL) return(e1NULLPIDPARM);
	spageid = *pageid;

#ifdef EVENTS 
	BF_event(procNum,"ReadBuf",&spageid,-1);
#endif
	BF_lock(&spageid);
	if ( (i = BF_lookup(&spageid)) == NIL) 
	{
		/* need a new buffer */
		if ((i = BF_allocbuf()) < eNOERROR)
                { 
			printf ("BF_allocbuf failled us.\n");
			BF_unlock(&spageid);
			return(i);
		}

		/* BF_allocbuf leaves the allocated buffer fixed */
		smPtr->buftable[i].Bfilenum = filenum;
		smPtr->buftable[i].Bfid = fid;
		smPtr->buftable[i].transId = transId;
		smPtr->buftable[i].Bpageid = spageid;
		smPtr->buftable[i].Bdirty = FALSE;

        	if ((e = io_readpage(&spageid,&(smPtr->bufferpool[i]))) < eNOERROR)
                { 
			BF_unlock(&spageid);
			SetLatch(&smPtr->bufTableLatch, procNum, NULL);
			smPtr->buftable[i].Bfixcnt = 0;
			smPtr->buftable[i].Brefbit = TRUE;
			smPtr->bf_num_free++;
			ReleaseLatch(&smPtr->bufTableLatch, procNum);
			return(e);
		}

		BF_insert(&spageid, i);	/* register it in the hash table */
#ifdef EVENTS 
		BF_event(procNum, "bf_rb1",&spageid,i);
#endif

#ifdef	TRACE
	if (checkset (&Trace1, tBUFMANAGER)) 
		printf("buffer %d, page %3.3d:%3.3d been swapped in\n",
			i, smPtr->buftable[i].Bpageid.Pvolid, spageid.Ppage);
#endif

	}
	else { if (smPtr->buftable[i].Bfilenum == -1) 
		{
		    smPtr->buftable[i].Bfilenum = filenum;	/* adopt the file */
		    smPtr->buftable[i].Bfid = fid;
		    smPtr->buftable[i].transId = transId;
		}
	        if (smPtr->buftable[i].Bfixcnt <=0) 
	        {
	            smPtr->buftable[i].Bfixcnt = 1;
	            smPtr->bf_num_free--;
	        }
	        else smPtr->buftable[i].Bfixcnt++;
#ifdef EVENTS
		BF_event(procNum, "bf_rb2",&spageid,i);
#endif
	}

	*returnpage = &(smPtr->bufferpool[i]);
	BF_unlock(&spageid);
	return(eNOERROR);

}  /* end bf_readbuf */

