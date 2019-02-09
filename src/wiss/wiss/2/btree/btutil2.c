
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
/* Module : Btree Utility Module (2)
  	This module contains the routines that deal with two B-tree pages. 
	They are used mainly for local reorganization during B-tree updates.
  
   IMPORTS:
  	io_freepage(fid, pid)
   	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
  	bf_freebuf(filenum, pageid, pageptr);
  	bf_setdirty(filenum, pageid, pageptr);
  	bt_compress_page(filenum, page)			from btutil1.c
  	bt_allocpage(trans_id, filenum, newpidptr, returnpage, keytype, pagetype,
  		indextype, unique?)
   EXPORTS:
  	bt_move_entries(trans_id, filenum, to_page, to_slot, 
  		from_page, from_slot, slot_count)
  	bt_split_page(trans_id, left_page, returnpage, new_slot, length)
  	bt_balance_pages(trans_id, left_page, right_page)
  	bt_merge_pages(trans_id, left_page, right_page)
*/

#include	<wiss.h>
#include	<st.h>
#include	<stdio.h>

bt_move_entries(trans_id, filenum, to_page, to_slot, from_page, from_slot, 
	slot_count)
int		trans_id;	/* transaction number */
int		filenum;	/* file number */
register BTREEPAGE *to_page;	/* which page to move to */
TWO		to_slot;	/* which slot on the page to move to */
register BTREEPAGE *from_page;	/* which page to move from */
TWO		from_slot;	/* which slot on the page to move from */
TWO		slot_count;	/* how many slots to move */

/* This routine moves entries from one page to another

   RETURNS:
  	None
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	e2BADSLOTCOUNT
  
   BUGS:
  	assumes the destination page has enough room for the new entries
*/
{
	register i;		/* loop index */
	TWO	entry_len;	/* length of a entry */
	TWO	byte_count;	/* how many bytes to move in total */

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL)) {
		printf("bt_move_entries(filenum=%d,to_page=0x%x,to_slot=%d,",
			filenum, to_page, to_slot);
		printf("from_page=0x%x,from_slot=%d,slot_count=%d)\n",
			from_page, from_slot, slot_count);
	}
#endif
#ifdef	DEBUG
	if (checkset(&Trace2, tMOVEENTRIES)) {
		printf(" bt_move_entries : before moving entries, ");
		printf("   the destination ="); bt_print_btpage(to_page);
		printf("     the source ="); bt_print_btpage(from_page);
	}
#endif

	if (slot_count < 0) return(e2BADSLOTCOUNT);

	/* calculate total # of bytes to move */
	for(i=from_slot, byte_count=0; 
		 i<from_slot+slot_count; i++,byte_count+=entry_len) {
		ENTRYLEN(from_page, i, entry_len);
		MAKEALIGN(entry_len);
		entry_len += (int) sizeof(from_page->slot[0]);
	}

	/* compress the destination page if necessary */
	if (USABLESPACE(to_page) < byte_count)
		(void) bt_compress_page(filenum, to_page);
	
	if (USABLESPACE(to_page) < byte_count) 
	{
	    (void) fprintf(stderr,
	     "WISS internal error: entry too large to fit in page (%d)\n",
	      byte_count);
	    return(e2KEYLENGTHTOOLONG);
	}

	/* make room in the slot array of the destination for the new entries */
	for(i = to_page->btcontrol.numoffsets - 1; i >= to_slot; i--)
		to_page->slot[-(i + slot_count)] = to_page->slot[-i];

	/* move the entries */
	for(i = 0; i < slot_count; i++) {
		to_page->slot[-(i+to_slot)] = to_page->btcontrol.enddata;
		ENTRYLEN(from_page, (i + from_slot), entry_len);
		movebytes(&to_page->data[to_page->btcontrol.enddata],
			ENTADDR(from_page, (i + from_slot)), entry_len);
		MAKEALIGN(entry_len);
		to_page->btcontrol.enddata += entry_len;
	}

	/* close the gap in the slot array of the souce page */
	for(i = from_slot + slot_count; i < from_page->btcontrol.numoffsets;i++)
		from_page->slot[-(i-slot_count)] = from_page->slot[-i];

	/* update the control info of both pages and set the buffers dirty */
	to_page->btcontrol.numfree -= byte_count;
	to_page->btcontrol.numoffsets += slot_count;
	from_page->btcontrol.numfree += byte_count;
	from_page->btcontrol.numoffsets -= slot_count;
	(void) bf_setdirty(filenum, &to_page->btcontrol.thispage, to_page);
	(void) bf_setdirty(filenum, &from_page->btcontrol.thispage, from_page);

#ifdef	DEBUG
	if (checkset(&Trace2, tMOVEENTRIES)) {
		printf(" bt_move_entries : after moving entries,");
		printf("   the destination ="); bt_print_btpage(to_page);
		printf("     the source ="); bt_print_btpage(from_page);
	}
#endif
#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("returning from bt_move_entries\n");
#endif

	return(eNOERROR);

} /* bt_move_entries */


bt_split_page(trans_id, filenum, left_page, returnpage, new_slot, length)
int		filenum;	/* file number */
BTREEPAGE	*left_page;	/* the page which is too crowded */
BTREEPAGE	**returnpage;	/* the newly allocated page */
TWO		new_slot;	/* the position of the new entry to be added */
TWO		length;		/* length of the new entry */

/* Split a B-tree page into two pages. Info on the new entry which 
  	causes the split (and has not been inserted yet) is given as 
  	parameters to help redistributing the entries more evenly.
  
   RETURNS:
  	buffer pointer of the newly allocated page
  
   SIDE EFFECTS:
	buffer for the new page is fixed
  
   ERRORS:
  	None
*/
{
	int		e;		/* for returned errors */
	TWO		i;		/* loop index */
	TWO		entry_length;	/* length of an entry */
	TWO		byte_count;	/* how many bytes involved */
	PID		pid;		/* PID of the newly allocated page */
	BTREEPAGE	*new_page;	/* buffer for the new page */
	BTREEPAGE	*next_page;	/* buffer for the next page */

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL)) {
	printf("bt_split_page(filenum=%d,left=0x%x,returnpage=0x%x,r_slot=%d,length=%d)\n",
			filenum, left_page, returnpage, new_slot, length);
	}
#endif

	/* determine where to split the page */
	byte_count = (USEDSPACE(left_page) + length) / 2;
	for (i = 0; byte_count > 0; i++) { /* how much should stay ? */
		if (new_slot == i && byte_count >= 0) {
			byte_count -= length + (int) sizeof(left_page->slot[0]);
			if (byte_count <= 0) break;
		}
		ENTRYLEN(left_page, i, entry_length);
		MAKEALIGN(entry_length);
		byte_count -= entry_length + (int) sizeof(left_page->slot[0]);
	}

	/* allocate a page (right page) */
	e = bt_allocpage(trans_id, filenum, &pid, &new_page, 
		left_page->btcontrol.keytype, left_page->btcontrol.pagetype, 
		INDEX, left_page->btcontrol.unique);
	CHECKERROR(e);

	/* return the buffer of the new page */
	*returnpage = new_page;	

	/* adjust the links in the adjacent 2 or 3 pages */
	new_page->btcontrol.prev = left_page->btcontrol.thispage.Ppage;
	new_page->btcontrol.next = left_page->btcontrol.next;
	left_page->btcontrol.next = pid.Ppage;
	if((pid.Ppage = new_page->btcontrol.next) != NULLPAGE) {
		/* read in the right page of the current one, and make its
             	   "prev" points to the new page instead of the current one */
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), 
			&pid, (PAGE **)&next_page);
		if (e < eNOERROR) {
			(void) bf_freebuf(filenum, 
			  &new_page->btcontrol.thispage, (PAGE *)new_page);
			return(e);
		}
		next_page->btcontrol.prev = new_page->btcontrol.thispage.Ppage;
		(void) bf_setdirty(filenum, &pid, next_page);
		(void) bf_freebuf(filenum, &pid, next_page);
	}

	/* move entires starting from i to the new page */
	e = bt_move_entries(trans_id, filenum, new_page, 0, left_page, i,
			left_page->btcontrol.numoffsets - i);

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("returning from bt_split_page\n");
#endif
	return(e);

} /* bt_split_page */


bt_balance_pages(trans_id, filenum, left_page, right_page)
int		trans_id;	/* transaction number */
int		filenum;	/* file number */
BTREEPAGE	*left_page;	/* page on the left */
BTREEPAGE	*right_page;	/* page on the right */

/* This routine balances the storage utilization of two neighboring pages.
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	None

   Bugs:
	assumes the pages are in the corrrect order
*/
{
	int		e;		/* for returned errors */
	TWO		i;		/* loop index */
	TWO		entry_len;	/* length of an entry */
	TWO		byte_count;	/* how many bytes involved */

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("bt_balance_pages(filenum=%d,left=0x%x,right=0x%x)\n",
			filenum, left_page, right_page);
#endif

	/* determine # of bytes each page should have */
	byte_count = (USEDSPACE(left_page) + USEDSPACE(right_page))/2;
	if (USEDSPACE(left_page) < USEDSPACE(right_page)) {
		/* borrow entries from the right neighbor */
		byte_count -= USEDSPACE(left_page); /* how much to borrow ? */
		for (i = 0; byte_count > 0; i++, byte_count -= entry_len)
					ENTRYLEN(right_page, i, entry_len);
		e = bt_move_entries(trans_id, filenum, left_page, 
	  		left_page->btcontrol.numoffsets, right_page, 0, i);
	}
	else { /* borrow entries from the left neighbor */
		byte_count -= USEDSPACE(right_page); /* how much to borrow ? */
		for (i = left_page->btcontrol.numoffsets - 1; 
				byte_count > 0; i--, byte_count -= entry_len)
			ENTRYLEN(left_page, i, entry_len);
		e = bt_move_entries(trans_id, filenum, right_page, 0, 
			left_page, i, left_page->btcontrol.numoffsets - i);
	}

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("returning from bt_balance_pages\n");
#endif

	return(e);

} /* bt_balance_pages */


bt_merge_pages(trans_id, filenum, left_page, right_page)
int		trans_id;	/* transaction number */
int		filenum;	/* file number */
BTREEPAGE	*left_page;	/* page on the left */
BTREEPAGE	*right_page;	/* page on the right */

/* This routine merges two neighboring B-tree pages. 
   Entries on the right page are moved to the page on the left and
   the page on the right is released.
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	None

   Bugs:
	assumes the pages are in correct order
*/
{
	int		e;	/* for returned errors */
	FID		fileid;	/* level o file id */
	PID		pid;	/* level 0 page id */

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("bt_merge_pages(filenum,left=0x%x,right=0x%x)\n",
			filenum, left_page, right_page);
#endif

	/* evacuate entries on the right page */
	e = bt_move_entries(trans_id, filenum, left_page,
		left_page->btcontrol.numoffsets,
		right_page, 0, right_page->btcontrol.numoffsets);
	CHECKERROR(e);

	/* extract the control info and releases the page on the right */
	fileid = right_page->btcontrol.fileid;
	pid = right_page->btcontrol.thispage;
	left_page->btcontrol.next = right_page->btcontrol.next;
	e = io_freepage(&fileid, &pid);
	CHECKERROR(e);

 	/* adjust the link in the (new) right neighbor */  
	if ((pid.Ppage = left_page->btcontrol.next) != NULLPAGE) {
		e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), 
			&pid, (PAGE **)&right_page);
		CHECKERROR(e);
		right_page->btcontrol.prev=left_page->btcontrol.thispage.Ppage;
		(void) bf_setdirty(filenum, &pid, right_page);
		(void) bf_freebuf(filenum, &pid, (PAGE *)right_page);
	}

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("returning from bt_merge_pages\n");
#endif
	return(eNOERROR);

} /* bt_merge_pages */

