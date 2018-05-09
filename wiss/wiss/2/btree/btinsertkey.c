
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
/* Module bt_insertkey 
  	The routine in this module inserts a key/pointer 
  	pair into a root or node page. If need be, the page is split
  	recursively; That is, node spliting may propagate upward
  	level by level util an ancestor has enough room to accomdate the
  	new key/ptr pair of its new son.
  	Prefix keys are used to increase the fanout of node pages.
  
   IMPORTS:
  	bf_readbuf(trans_id, filenum, fid, pageid, returnpage)
  	bt_binary_search(key_len, key, page, slotnum) 	from btutil1.c
  	bt_allocpage(trans_id, filenum, newpidptr, returnpage, keytype, 
  				pagetype, treetype, unique) 	from btutil1.c
  	bt_insertentry(filenum, btpage, length, entry, slotnum)	from btutil1.c
  	bt_move_entries(trans_id, filenum, to_page, to_slot, from_page, from_slot
  		slot_count)					from btutil2.c
  	bt_split_page(trans_id filenum, old_page, new_page@)	from btutil2.c
  	bt_prefixkey(filenum, page, slotnum, k_len, k_val, 
		trans_id, lockup, cond)	
								from btutilx.c
  
   EXPORTS:
  	bt_insertkey(filenum, pl, pi, k_len, k_val, pointer, trans_id, 
		lockup, cond)
*/

#include <wiss.h>
#include <st.h>

bt_insertkey(filenum, pl, pi, k_len, k_val, ptr, trans_id, lockup, cond)
int		filenum;	/* open file number of the index file */
PARENTLIST	pl;		/* node to be inserted in is at	*/
PARENTINDEX	*pi;		/* top of parentlist "stack"	*/
TWO		k_len;		/* length of the given key 	*/
char		*k_val;		/* actual key to be inserted	*/
PID		*ptr;		/* pointer part of key/ptr pair	*/
int             trans_id;
short           lockup;		/* if TRUE the pages are individually locked */
short           cond;		

/* DESCRIPTION:
  	Insert a key/pointer pair into the node at the top of the parentlist.
  
   RETURNS:
	None
  
   SIDE EFFECTS:
  	Parent_list may be modified as a result of node spliting
  
   ERRORS:
  	e2DUPLICATEKEYPTR - key already exist in the node (system inconsistency)
  
   BUGS:
  	No protection against parent list overflow
*/
{
	int		e;		/* error code			*/
	BTREEPAGE	*node;		/* on which to insert		*/
	BTREEPAGE	*new_page;	/* new page for spliting	*/
	TWO		slotnum;	/* where to insert		*/
	TWO		i;		/* temp variable */
	TWO 		entry_len;	/* length of the new key/ptr	*/
	PID		new_pid;	/* page id of the new page 	*/
	char	buff[MAXKEYLEN+sizeof(k_len)+sizeof(SHORTPID)];
	char	keybuff[MAXKEYLEN];
	TWO	dataneeded;

	(*pi)--;	/* need BEFORE trace! the pid to insert into is not 
			   really on top of the parent list, but is second, so 
			   we decrement here then increment just before 
			   returning.  */
#ifdef	TRACE
	if (checkset(&Trace2, tINSERTKEY)) {
		printf("~bt_insertkey(filenum=", filenum);
		printf(", ptr="); PRINTPIDPTR(ptr); printf(", ");
		PRINTPARENTLIST(pl, *pi); printf(")\n");
	}
#endif

	if (k_len >= MAXKEYLEN) {
		return (e2KEYLENGTHTOOLONG);
	}


	/* save the key & page pointer in local variables */
	for (i = 0; i < k_len; i++) keybuff[i] = *(k_val++);
	new_pid = *ptr;

	/* read in the node page on which the key/ptr is to be inserted */
	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pl[*pi].page_id, (PAGE **)&node);
	CHECKERROR(e);

	/* locate the position of the new key/ptr on the page, return error if
	   the key already exists (system inconsistency !) */
	e = bt_binary_search(k_len, keybuff, node, &slotnum);
	if (e >= eNOERROR) {
		(void) bf_freebuf(filenum, &pl[*pi].page_id, (PAGE *)node);
#ifdef	DEBUG
		bt_print_btpage(node);	
#endif
		return(e2DUPLICATEKEYPTR);
	}

	/* for string keys, find the shorest proper prefix of the given key 
	   that is not a proper prefix of the index right before it */
	if (node->btcontrol.keytype == (int) TSTRING) {
		k_len = bt_prefixkey(filenum, node, slotnum, k_len, keybuff, 
			trans_id, lockup, cond);
		if (k_len < eNOERROR) {
			(void) bf_freebuf(filenum, &pl[*pi].page_id, (PAGE *)node);
			return(k_len);
		}
	}
	
	/* Check that there is enough room on the page. If there is, insert 
	   the key/ptr pair directly into the page; otherwise split the node 
	   and insert the key (recursively).  */

	MAKEKEYPTR(buff, k_len, keybuff, new_pid, entry_len);
	dataneeded = entry_len;
	MAKEALIGN(dataneeded);
	dataneeded += (int)sizeof(node->slot[0]);
	if (dataneeded <= node->btcontrol.numfree) {
		/* insert the key/ptr pair into this page and return */
		(void) bt_insertentry(filenum, node, entry_len, buff, slotnum);
		(void) bf_freebuf(filenum, &pl[*pi].page_id, (PAGE *)node);
#ifdef	TRACE
		if (checkset(&Trace2, tINSERTKEY)) {
			printf(" return from bt_insertkey at level %d\n", *pi);
			if (checkset(&Trace2, tTREEDUMP))
				bt_print_btfile(filenum, &(pl[*pi].page_id), trans_id, FALSE);
		}
#endif
		(*pi)++;

		return(eNOERROR);

	} /* end of simple key/ptr insertion */

/***************************************************************************/
/* Well! since simple insertion is not possible, here comes the hard part. */
/***************************************************************************/

/* Split the node which is too crowded. (ROOT is treated differently!)
   To make spliting the root easier, a new page is allocated first to 
   inherit all the entries from the root, and let the new page be 
   pointed to by the PID0 of the root. Then the page is split as usual.
   Although inefficient, root spliting should NOT be done very often */

	if (node->btcontrol.pagetype == ROOTPG) {  /* root spliting ! */
		e = bt_allocpage(trans_id, filenum, &new_pid, &new_page, 
			node->btcontrol.keytype, NODEPG, 
			node->btcontrol.treetype, node->btcontrol.unique);
		if (e < eNOERROR) {
			(void) bf_freebuf(filenum, &pl[0].page_id, (PAGE *)node);
			return(e);
		}
		F_NUMPAGES(filenum)++, F_STATUS(filenum) = DIRTY;

		/* move all the entries from root to the new page */
		(void) bt_move_entries(trans_id, filenum, new_page, 0, node, 0, 
			node->btcontrol.numoffsets);

		/* update the B-tree hierarchy and update the parent list */
		new_page->btcontrol.pid0 = node->btcontrol.pid0;
		node->btcontrol.pid0 = new_pid.Ppage;
		for (i = MAXPARENTS - 1; i > 1; i--) 
		{
		    pl[i].page_id = pl[i-1].page_id;
		    pl[i].locked = pl[i-1].locked;
		}
		pl[1].page_id = new_pid;
		(*pi)++;

		(void) bf_freebuf(filenum, &pl[0].page_id, (PAGE *)node);
		node = new_page;	/* split this new page now */
	}  /*  ---  end of special processing for root spliting  ---  */

	/* Split the current page into two evenly distrubuted pages */
	e = bt_split_page(trans_id, filenum, node, &new_page, 
		slotnum, entry_len);
	if (e < eNOERROR) {
		(void) bf_freebuf(filenum, &pl[*pi].page_id, (PAGE *)node);
		return(e);
	}
	F_NUMPAGES(filenum)++, F_STATUS(filenum) = DIRTY;
	new_pid = new_page->btcontrol.thispage;

	/* Determine which page the passed-in key should go on.  Since the
	   parent list has the original left page on the top of it, if the key 
	   is to go on the new page the list will have to be updated 
	*/
	i = node->btcontrol.numoffsets;
	if ((slotnum < i) || slotnum==i && FREESPACE(node)>FREESPACE(new_page))
		(void) bt_insertentry(filenum, node, entry_len, buff, slotnum);
	else {
		(void) bt_insertentry(filenum, new_page, entry_len, buff, slotnum - i);
		pl[*pi].page_id = new_pid;
	}

	/* Finally, the key/ptr pair of the new right node has to be 
	   inserted into its parent (recursively !).  */

	e = bt_insertkey(filenum, pl, pi, KEYLEN(new_page, 0),
		KEYVALUE(new_page, 0), &new_pid, trans_id, FALSE, cond);

	(void) bf_freebuf(filenum, &node->btcontrol.thispage, (PAGE *)node);
	(void) bf_freebuf(filenum, &new_page->btcontrol.thispage, (PAGE *)new_page);

#ifdef	TRACE
	if (checkset(&Trace2, tINSERTKEY))
		printf(" returning from bt_insertkey at level %d\n", *pi);
#endif

	(*pi)++;
	return(e);

} /* bt_insertkey */

