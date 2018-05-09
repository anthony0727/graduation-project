
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
/* Module r_dumpfile: print the control info of a file

   IMPORTS:
	io_readpage(pageid, pageptr)
	r_dumppage(pageptr, trans_id, lockup, cond)

   EXPORTS:
	r_dumpfile(filenum, trans_id, lockup, cond)

*/


#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

r_dumpfile(filenum, trans_id, lockup, cond)
int	filenum;
int     trans_id;    /* transaction id of the resource requesting locking */
short   lockup;      /* If lockup is true lock the page of behalf of trans */
short   cond;      

{
	DATAPAGE	page;
	FID		fileid;
	PID		first, last, tpid;
	int		e;

	fileid = FC_FILEID(filenum);
	/* Lock the file if not already locked */
	if (lockup)
	{
	    e = lock_file (trans_id, fileid, l_S, COMMIT, cond);
	    CHECKERROR(e);
	}

	printf("----------------------------------------------------------\n");
	printf("FILEID {%d,%d}", fileid.Fvolid, fileid.Ffilenum);

        first = F_FIRSTPID(filenum);
        last = F_LASTPID(filenum);

	printf(" first PID : {%d,%d}, last PID : {%d, %d} \n",
		first.Pvolid, first.Ppage, last.Pvolid, last.Ppage);

	/* flush the buffer but don't close it */
	bf_flushbuf(filenum, FALSE);

	for (tpid = first; tpid.Ppage != NULLPAGE; tpid.Ppage = page.nextpage) {
		(void) io_readpage(&tpid, &page);
		printf(" page %d:%d in file %d:%d, ",
			page.thispage.Pvolid, page.thispage.Ppage,
			page.fileid.Fvolid, page.fileid.Ffilenum);
		printf("Prev=%d, Next=%d, free=%d, # of slots = %d\n",
			page.prevpage, page.nextpage, page.free, page.ridcnt);

#ifdef	DEBUG
		r_dumppage(&page, trans_id, lockup);
#else
	{ 	int	s;
		RID	*rid;
		RECORD	*r;
		for (s = 0; s < page.ridcnt; s++) {
printf(" slot %d offset is %d - ", s, page.slot[-s]);
			if (page.slot[-s] == EMPTYSLOT) {
				printf(" slot %2d empty\n", s);
				continue;
			}
			r = (RECORD *) &(page.data[page.slot[-s]]);
			if (r->type != MOVED) 
				printf(" slot %2d[%3d]\n", s, r->length);
			else {
				rid = (RID *) r->data;
				printf(" slot %2d[%3d](moved):RID={%d:%d:%d}\n",
				s,r->length,rid->Rvolid,rid->Rpage,rid->Rslot);
			}
		}
	}
#endif
	}

	printf("----------------------------------------------------------\n");
}

