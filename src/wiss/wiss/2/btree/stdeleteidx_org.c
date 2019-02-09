
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



/* Module : This module contains the routine to delete an index from a B-tree.
  
   IMPORTS:
  	io_freepage(fid, pid);
  	bf_readbuf(transid, filenum, fid, pageid, returnpage)
  	bf_setdirty(filenum, pageid, pageptr)
  	bf_freebuf(filenum, pageid, pageptr)
  	bt_binary_search(key_len, key_value, page, slot@);
  	bt_deleteentry(filenum, page, slot)
  	bt_insertkey(filenum, parentlist, parentindex, key_length, key_value,
  			ptrpid, trans_id, lockup, cond)
  	bt_move_entries(trans_id, filenum, to_page, to_slot, 
			from_page, from_slot, count)
	bt_balance_pages(trans_id, filenum, pageptr1, pageptr2);
  	bt_traverse(filenum, key, leaf@, slot@, pindex@, parentlist@, trans_id,
		    lockup, operation_requested, cond);
  	bt_zaprid(filenum, leaf, slotnum, ridptr, trans_id, lockup, cond);
	clean_locks (parentlist, pindex, trans_id);
  
   EXPORTS:
  	st_deleteindex(filenum, key, RID, trans_id, lockup, cond);
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>


#define	BTCHECKERROR(e) if (e < eNOERROR) goto error;

st_deleteindex(filenum,key,ridptr, trans_id, lockup, cond)
int	filenum;		/* open file number of the B-tree file */
KEY	*key;		/* pointer to key */
RID	*ridptr;	/* pointer to the rid which are to be deleted */
int     trans_id;
short   lockup;
short   cond;

/* This routine deletes an index from a B-tree index file.
  	
   RETURNS:
  	eNOERROR if successful
  
   SIDE EFFECTS:
  	The whole key is also removed if the RID just deleted is the last one 
  	on its RID list.
  
   ERRORS:
  	e2NORIDMATCH : the RID is not on the RID list of the sepcified key
*/

{
	int		e;		/* for returned error codes */
	BTREEPAGE	*leaf;		/* leaf page to delete key from */
	TWO		slot_num;	/* position of the key */
	PARENTINDEX	pindex;		/* index for parentlist */
	PARENTLIST	parentlist;	/* parent stack for bt_traverse */

#ifdef	TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_deleteindex(filenum = %d, key = ", filenum);
		PRINTKEY(key);
		printf(", RID = "); PRINTRIDPTR(ridptr); printf(")\n");
	}
#endif

	CHECKOFN(filenum);
	CHECKWP(filenum);

	/* locate the key (page, slot number) */
	/* lock the pages if overflow or underflow occurs since  */
	/* bt_reorganize will try to insert entries */

	e = bt_traverse(filenum, key, &leaf, &slot_num, &pindex, 
		parentlist, trans_id, lockup, BT_INSERT_DELETE, cond);

	if (e == ABORTED)
	    return(e);

#ifdef DEBUG
	if (checkset(&Trace2, tBTREE))
		if (e < eNOERROR) bt_print_btpage(leaf);
#endif

	BTCHECKERROR(e);

	/* remove the RID from the list */
	e = bt_zaprid(filenum, leaf, slot_num, ridptr, trans_id, lockup, cond);
	BTCHECKERROR(e);

	/* rebalance the leaf pages if necessary */
	if (USEDSPACE(leaf) < MAXSPACE(leaf)/2) 
		e = bt_reorganize(filenum, pindex, parentlist, leaf, trans_id,
				  lockup, cond);

error:
	(void) bf_freebuf(filenum, &leaf->btcontrol.thispage, (PAGE *)leaf);
	if (lockup)
	{
	    clean_locks (parentlist, pindex, trans_id);
	}
	return(e);

} /* st_deleteindex */


bt_reorganize(filenum, parentindex, parentlist, leaf, trans_id, lockup, cond)
int		filenum;
PARENTINDEX	parentindex;	/* depth of the parent stack */
PARENTLIST	parentlist;	/* parent stack */
BTREEPAGE	*leaf;		/* the leaf where the deleted key was on */
int             trans_id;
short           lockup;		/* lock the page if lockup is TRUE */
short           cond;		/* if TRUE, conditional locks are used */

/* This routine check if the given leaf page is less then half full,
  	if so, it reorganize (recursively) the tree either by merging pages 
  	or redistributing entries on neighboring pages.
  	For efficiency, the recursive process is implemented by iteration.
  	
   RETURNS:
	None
  
   SIDE EFFECTS:
  	Since prefix keys are used, some higher nodes may be split 
  	(recursively) as a result of redistribution
  	parent list may be invalidated
  
   ERRORS:
  	None
  
   BUGS:
  	None
*/
{
	int		e;		/* for returned error codes */
	BTREEPAGE	*cur_node;	/* page that may be less than 1/2 full */
	BTREEPAGE	*nb_node;	/* neighbor that can help */
	BTREEPAGE	*p_node;	/* parent of the above pages */
	BTREEPAGE	*t_node;	/* for variables swapping */
	TWO		cur_slot;	/* slot # of current page in parent */
	TWO		nb_slot;	/* slot # of the neighbor in parent */
	TWO		t_slot;		/* for variables swapping */
	TWO		key_length;	/* key length */
	FID		fid;		/* level 0 file id */
	PID		pid;

#ifdef	TRACE
	if (checkset(&Trace2, tREORGANIZE)) {
		printf("bt_reorganize(parent list =");
		PRINTPARENTLIST(parentlist, parentindex);
		printf(", leaf=0x%x)\n", leaf);
	}
#endif

	/* get level 0 file id */
	fid = leaf->btcontrol.fileid;
	
	/* in each iteration of the following loop, cur_node is the page
	   that may be less that half full, nb_node is the chosen neighbor
	   to participate in the local reorganiztion, p_node is their parent 
	*/
	for (cur_node = leaf, nb_node = p_node = NULL;
		USEDSPACE(cur_node) < MAXSPACE(cur_node)/2 && parentindex > 0; 
		(cur_node = p_node, nb_node = p_node = NULL), parentindex--) 
	{

	    /* read the parent page in */
	    pid = parentlist[parentindex - 1].page_id;
	    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&p_node);
	    BTCHECKERROR(e);
	    if (p_node->btcontrol.numoffsets == 0)  break;/* no neighbors */

	    /* locate position of the current page in the parent node */
	    e = bt_binary_search(KEYLEN(cur_node, 0), KEYVALUE(cur_node, 0),
			p_node, &cur_slot);
	    if (e == e2KEYNOTFOUND) cur_slot--;

	    /* select and read in the neighbor */
	    nb_slot = cur_slot + /* prefere the right neighbor */
	    	((cur_slot < p_node->btcontrol.numoffsets - 1) ? 1:-1);
		pid.Ppage = (nb_slot < cur_slot) ? 
			cur_node->btcontrol.prev : cur_node->btcontrol.next;

/*
 *  Since actual update is going to take place on pid lock the page in
 *  eXclusive mode, but dont forget to unlock the page at the end of the
 *  for loop.
 */
	    if (lockup)
	    {
	        e = lock_page(trans_id, FC_FILEID(filenum), pid, l_X,
			MANUAL, cond);
	        if (e == ABORTED) return (ABORTED);

	    }
	    e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &pid, 
		(PAGE **)&nb_node);
	    BTCHECKERROR(e);

	    /* here is the real reorganization starts */
	    /* to simplify the processing, make cur_node always be the
	     page on the left; and nb_node be the page on the right.
	     switch the roles of cur_node and nb_node if necessary */

	    if (nb_slot < cur_slot) 
	    {
	    	t_slot = nb_slot; nb_slot = cur_slot; cur_slot = t_slot;
		t_node = nb_node; nb_node = cur_node; cur_node = t_node;
	    }

	    if (cur_node->btcontrol.numfree > USEDSPACE(nb_node)) 
	    {
		/* these two pages mergable */
		(void) bt_merge_pages(trans_id, filenum, cur_node, nb_node);
		F_NUMPAGES(filenum)--, F_STATUS(filenum) = DIRTY;
		(void) bt_deleteentry(filenum, p_node, nb_slot);
	    }
	    else 
	    {   /* pages not mergable, redistribute entries */
	        (void) bt_balance_pages(trans_id, filenum, cur_node, nb_node);
	        /* update the key/ptr in parent node */
	        key_length = KEYLEN(nb_node, 0);
	        if (p_node->btcontrol.keytype != (int) TSTRING) 
	        {
		    movebytes(KEYVALUE(p_node, nb_slot),
			KEYVALUE(nb_node, 0), key_length);
		    (void) bf_setdirty(filenum, &p_node->btcontrol.thispage, 
			p_node);
		}
		else 
		{
		    (void) bt_deleteentry(filenum, p_node, nb_slot);
		    e = bt_insertkey(filenum, parentlist, &parentindex,
			key_length, KEYVALUE(nb_node, 0),
			&(nb_node->btcontrol.thispage), trans_id, FALSE, cond);
		    BTCHECKERROR(e);
		 }
	    }

	    if (p_node->btcontrol.pagetype == ROOTPG) break;
	    /* be sure not to release the buffer of the leaf page! */
	    if (cur_node != leaf)
	        (void) bf_freebuf(filenum, &cur_node->btcontrol.thispage, 
			(PAGE *)cur_node);
	    if (nb_node != leaf)
	        (void) bf_freebuf(filenum, &nb_node->btcontrol.thispage, 
			(PAGE *)nb_node);
/*
 *  Unlock the page which you locked at the top.
 */
	    if (lockup) m_release_page(trans_id, pid);

	} /* end for */

	/* if the root has only one non-leaf child who can fit into the
	   root, move entries from the child and release it,
   	   the height of the tree is thus decremented by 1 */
	if (p_node != NULL && p_node->btcontrol.pagetype == ROOTPG
	    && cur_node->btcontrol.pagetype != LEAFPG
	    && p_node->btcontrol.numoffsets == 0
	    && MAXSPACE(p_node) > USEDSPACE(cur_node)) { 
		e = bt_move_entries(trans_id, filenum, p_node, 0,cur_node,
			0, cur_node->btcontrol.numoffsets);
		p_node->btcontrol.pid0=cur_node->btcontrol.pid0;
		e = io_freepage(&fid, &(cur_node->btcontrol.thispage));
		BTCHECKERROR(e);
		F_NUMPAGES(filenum)--, F_STATUS(filenum)=DIRTY;
		(void) bf_setdirty(filenum, &p_node->btcontrol.thispage, p_node);
	}

	/* be sure not to release the buffer of the leaf page! */
error:
	if (cur_node != NULL && cur_node != leaf)
	    (void) bf_freebuf(filenum,&cur_node->btcontrol.thispage,(PAGE *)cur_node);
	if (nb_node != NULL && nb_node != leaf) 
		(void) bf_freebuf(filenum,&nb_node->btcontrol.thispage,(PAGE *)nb_node);
	if (p_node != NULL)
		(void) bf_freebuf(filenum, &p_node->btcontrol.thispage, (PAGE *)p_node);
	
#ifdef	TRACE
	if (checkset(&Trace2, tREORGANIZE)) {
		printf("returning from bt_reorganize\n");
#ifdef	DEBUG
	if (p_node != NULL && checkset(&Trace2, tTREEDUMP))
		(void) bt_print_btfile(filenum, &(p_node->btcontrol.thispage), 
			trans_id, lockup, cond);
#endif

	}
#endif

	return(e);

} /* bt_reorganize */

