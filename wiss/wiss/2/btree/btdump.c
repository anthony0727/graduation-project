
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
/* Module bt_dump: Btree Debugging Utility Module
  	This module contains dubugging utilities for B-tree routines.

   Imports:
	bf_readbuf(transId, filenum, fid, pageid, returnpage)
	bf_freebuf(filenum, pageid, pageptr)
	r_getrecord(filenum, ridptr, returnpage, recptr, trans_id, lockup, mode, 		cond)
	st_firstfile(filenum, firstrid, trans_id, lockup, mode, cond)
	st_nextfile(filenum, currid, enxtrid, trans-id, lockup, mode, cond);
	st_readframe(filenum, dirridptr, offset, buf, length, lockup, 
		mode, cond)
  
   Exports:
  	bt_print_btpage(btpage);
  	bt_print_overflow(filenum, ov_rid);
  	bt_print_btfile(filenum, rootpid, trans_id, lockup, cond);
  	bt_print_keyridfile(filenum, keyattr, trans_id, lockup, cond);

*/
  
#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

static 
print_page(page)
register BTREEPAGE	*page;

/* This routine prints the contents of a B-tree page.
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	None
*/
{
	register 	i, j;		/* loop indices */
	TWO		count;		/* rid count must be type TWO */
	char		*ptr;		/* RID list pointer */
	RID		trid;		/* temp RID for print */
	SHORTPID	spid;		/* short PID in key/ptr */

	if (page->btcontrol.numoffsets == 0) printf(" an empty B-tree page\n");

	for (i = 0; i < page->btcontrol.numoffsets; i++) {
		printf("    slot[%2.2d] (at %4.4d): ", i, page->slot[-i]);
		if (page->slot[-i] == EMPTYSLOT) continue;
		/* print the key */
		printf("key length = %2d, key = ", KEYLEN(page, i));
		print_data(page->btcontrol.keytype, KEYLEN(page, i),
				KEYVALUE(page, i));
		if (page->btcontrol.pagetype == LEAFPG) { /* print RID list */
			MOVERIDCOUNT((char *)&count, RIDCOUNT(page, i));
			ptr = RIDLIST(page, i); 
			MOVERID((char *)&trid, ptr);
			if (count < 0) {
				printf("\n\tOverflow list (count=%d)", -count);
				printf(" directory={%d:%d:%d}\n",
					trid.Rvolid, trid.Rpage, trid.Rslot);
			}
			else if (count == 1) printf(" Rid={%d:%d:%d}", 
					trid.Rvolid, trid.Rpage, trid.Rslot);
			else {
				printf("\n\tRID count=%d, ", count);
				printf(" RID list: ");
				for (j=0; j<count; j++, ptr+=sizeof(RID)) {
					printf("%s", (j%4) == 0 ? "\n\t" : ",");
					MOVERID((char *)&trid, ptr);
					printf(" rid[%d]: {%d,%d,%d}", 
					j, trid.Rvolid, trid.Rpage, trid.Rslot);
				}
			}
			printf("\n");
		}
		else { /* print the pointer associated with the key */
			GETPTR(page, i, spid);
			printf(", PTR = %d\n", spid); 
		}
	}

} /* print_page */


bt_print_btpage(btpage)
register BTREEPAGE	*btpage;

/* This routine prints the header information and the contents of a B-tree page
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	None
*/
{
	BTCONTROL	*p;

	printf("\n - [| Control Information (for page at 0x%x)  |]", btpage);
	printf("  --------------------\n");
	p = &(btpage->btcontrol);
	printf("PID=[%d:%d],",p->thispage.Pvolid, p->thispage.Ppage);
	printf("FILEID=[%d:%d],",p->fileid.Fvolid,p->fileid.Ffilenum);
	printf("NEXT=[%d],PREV=[%d],OVERFLOW=[%d],PID0=[%d]\n",
		p->next, p->prev, p->overflow , p->pid0);
	printf(" enddata=%d,startdata=%d,numfree=%d,numoffsets=%d,", 
		p->enddata, p->startdata, p->numfree, p->numoffsets);
	printf("keytype=%d,pagetype=%c,treetype=%c,", 
		p-> keytype, p->pagetype, p->treetype);
	printf("unique=%s\n", p->unique == (ONE)TRUE ? "true" : "false");

	switch(btpage->btcontrol.pagetype) {
	  case ROOTPG:
		printf("Index Attribute = ");
		PRINTATTR((KEYINFO *)(btpage));
		printf("\n");
	  case NODEPG:
	  case LEAFPG:
	  case OVFLPG:
		print_page(btpage);
		break;
	  default:
		printf("\tBad case label in bt_print_btpage: %d\n",
			(int) btpage->btcontrol.pagetype);
	}

} /* bt_print_btpage */


bt_print_overflow(filenum, ridptr)
int	filenum;	/* open file # of the B-tree */
RID	*ridptr;	/* directory of the overflow list */

/* This routine prints an overflow RID list
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	None

*/
{
	int	j;	/* loop index */
	RID	trid;	/* temp RID */
	int 	e;	/* for returned errors */

	printf("\tdirectory= ");
	PRINTRIDPTR(ridptr);
	printf(" RID list: ");
	for (j = 0; ; j++) {
		printf("%s", (j % 4) == 0 ? "\n        " : ",");
		e = st_readframe(filenum, ridptr, j*sizeof(RID),
			(char *)&trid, sizeof(RID), -1, FALSE, FALSE);
		if (e < sizeof(RID)) break;
		printf(" rid[%d]:{%d,%d,%d}", 
			j, trid.Rvolid, trid.Rpage, trid.Rslot);
	}
	printf("\n");

} /* bt_print_overflow */


bt_print_levels(filenum, rootpid, trans_id, lockup, cond)
int	filenum;	/* open file # of the B-tree file */
PID	*rootpid;	/* Pid of the root page */
int     trans_id;
short   lockup;
short	cond;

/* This routine prints all the pages of a btree rooted at a given page.
  
   RETURNS:
	None
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	None
*/
{
	int		e;		/* for returned errors */
	int		level;		/* level # down the tree */
	PID		pid;		/* pid of the current page */
	RID		currid;
	SHORTPID	pid0;		/* first page of the next level */
	SHORTPID	nextpid;
	BTREEPAGE	*btpage;		

	if (rootpid != NULL) pid = *rootpid;	/* start from this sub-tree */
	else pid = F_ROOTPID(filenum);		/* start from the root */

	for (level = 0; pid.Ppage != NULLPAGE; level++) {
		printf(" * Page(s) at level %d : *", level);
		/* read in the first page at this level */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
			(PAGE **) &btpage);
		CHECKERROR(e);
		for (pid0 = btpage->btcontrol.pid0; pid.Ppage != NULLPAGE;) {
			nextpid = btpage->btcontrol.next;
			(void) bf_freebuf(filenum, &pid, (PAGE *)btpage);
			if ((pid.Ppage = nextpid) != NULLPAGE) {	
				e = bf_readbuf(trans_id, filenum, 
					FC_FILEID(filenum), &pid,
					(PAGE **)&btpage);
				CHECKERROR(e);
			}
		}
		pid.Ppage = pid0;	/* first page of the next level */
	}

	printf(" ----- Overflow RID Lists ------ \n");
	for (e = st_firstfile(filenum, &currid, trans_id, lockup, l_S, cond); 
	     e >= eNOERROR; ) 
	{
		bt_print_overflow(filenum, &currid);
		e = st_nextfile(filenum, &currid, &currid,
		  trans_id, lockup, l_S, cond);
	}
	printf(" ------------------------------------------- \n");

	return(eNOERROR);

} 


bt_print_btfile(filenum, rootpid, trans_id, lockup, cond)
int	filenum;	/* open file # of the B-tree file */
PID	*rootpid;	/* Pid of the root page */
int     trans_id;
short   lockup;
short   cond;

/* This routine prints all the pages of a btree rooted at a given page.
  
   RETURNS:
	None
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	None
*/
{
	int		e;		/* for returned errors */
	int		level;		/* level # down the tree */
	PID		pid;		/* pid of the current page */
	RID		currid;
	SHORTPID	pid0;		/* first page of the next level */
	SHORTPID	nextpid;
	BTREEPAGE	*btpage;		

	if (rootpid != NULL) pid = *rootpid;	/* start from this sub-tree */
	else pid = F_ROOTPID(filenum);		/* start from the root */

	for (level = 0; pid.Ppage != NULLPAGE; level++) {
		printf(" * Page(s) at level %d : *", level);
		/* read in the first page at this level */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, (PAGE **) &btpage);
		CHECKERROR(e);
		for (pid0 = btpage->btcontrol.pid0; pid.Ppage != NULLPAGE;) {
			bt_print_btpage(btpage);
			nextpid = btpage->btcontrol.next;
			(void) bf_freebuf(filenum, &pid, (PAGE *)btpage);
			if ((pid.Ppage = nextpid) != NULLPAGE) {	
				e = bf_readbuf(trans_id, filenum,FC_FILEID(filenum), &pid,(PAGE **)&btpage);
				CHECKERROR(e);
			}
		}
		pid.Ppage = pid0;	/* first page of the next level */
	}

	printf(" ----- Overflow RID Lists ------ \n");
	for (e = st_firstfile(filenum, &currid, trans_id, lockup, l_S, cond); 
		e >= eNOERROR; ) 
	{
		bt_print_overflow(filenum, &currid);
		e = st_nextfile(filenum,&currid,&currid, 
		    trans_id, lockup, l_S, cond);
	}
	printf(" ------------------------------------------- \n");

	return(eNOERROR);

} /* bt_print_btfile */


bt_print_keyridfile(filenum, keyattr, trans_id, lockup, cond)
int		filenum;	/* open file num of keyridfile */
KEYINFO		*keyattr;	/* ptr to key attr info */
int             trans_id;
short           lockup;
short		cond;

/* This routine prints the key and rid entries in a keyridfile.
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	None
*/
{
	int		e;		/* error code return		*/
	RID		currid;		/* rid of the current key-rid record */
	RID		trid;		/* used for printing RID */
	RECORD  	*recptr;	/* ptr for record address	*/
	DATAPAGE	*dp;		/* pointer to a data page */

	printf(" DUMP of The KEY-RID FILE\n");
	for (e = st_firstfile(filenum, &currid, trans_id, lockup, l_S, cond); 
	 	e >= eNOERROR; ) 
	{
		/* firstfile/nextfile leave the page locked in S mode */
		e = r_getrecord(filenum, &currid, &dp, &recptr, 
			trans_id, FALSE, l_NL, cond);
		CHECKERROR(e);
		MOVERID((char *)&trid, &(recptr->data[keyattr->length])); 
		printf(" (key;rid) = (key length=%2d, key=", keyattr->length);
		print_data(keyattr->type, keyattr->length, recptr->data);
		printf("; RID = "); PRINTRID(trid); printf(")\n");
/*
  don't free page so nextfile can do its thing 
		(void) bf_freebuf(filenum, &dp->thispage, (PAGE *)dp);
*/
		e = st_nextfile(filenum,&currid,&currid, 
			trans_id, lockup, l_S, cond);
	}
	printf("\n");

	return(eNOERROR);

} /* bt_print_keyridfile */

