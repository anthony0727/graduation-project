
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
/* Module r_dumppage: print the contents of a page 

   IMPORTS:
	None

   EXPORTS:
	r_dumppage(D, trans_id, lockup)
*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


static printslot(D, s)
register DATAPAGE	*D;	/* pointer to page whose slot we print */
int		s;	/* slot number */
{
	register char	*str;
	register	i;
	RID		*rid;
	RECORD		*r = (RECORD *) &(D->data[D->slot[-s]]);

	if (r->type != MOVED) {
		printf(" slot %2d[%3d]:", s, r->length);
		for (i = 0, str = r->data; i < r->length; i++, str++)
			printf("%c", *str);
		printf("\n");
	}
	else {
		rid = (RID *) r->data;
		printf(" slot %2d[%3d](moved):RID={Volid=%d,Page=%d,Slot=%d}\n", 
			s, r->length, rid->Rvolid, rid->Rpage, rid->Rslot);
	}
}

r_dumppage(D, trans_id, lockup, cond)
register DATAPAGE	*D;
int      trans_id;             
short    lockup;              
short	 cond;
{
	register int	s;
	int e;

      /* if page is not locked, lock it in shared mode */
      if (lockup) {
	      e = lock_page(trans_id, D->fileid, D->thispage, l_S, COMMIT, 
		cond);
	      CHECKERROR(e);
      }

	printf("\nData Page %d:%d in file %d:%d; ",
		D->thispage.Pvolid, D->thispage.Ppage,
		D->fileid.Fvolid, D->fileid.Ffilenum);
	printf("Ridcnt=%d, Prevpage=%d, Nextpage=%d, Freepointer=%d\n",
		D->ridcnt, D->prevpage, D->nextpage, D->free);
	printf("-----------------------------------------------------------------\n");
	printf("[slot,offset]:");
	for (s = 0; s < D->ridcnt; s++)
		printf("[%2d,%3d]%c", s, D->slot[-s], (s%10==9)?'\n':',');
	printf("\n");
	for (s = 0; s < D->ridcnt; s++)
		if (D->slot[-s] != EMPTYSLOT) printslot(D, s);
		else printf(" slot %2d[empty]\n", s);
	printf("-----------------------------------------------------------------\n");
}
