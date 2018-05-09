
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
/* Module : st_insertindex
  	This module contains the routine to insert an index into a btree.

   IMPORTS:
  	bf_setdirty(filenum, pageid, pageptr)
  	bt_insertentry(btpage, length, entry, slotnum);     
  	bt_insertkey(filenum, parentlist, paren_index, key_length,
  	       key_value, pageid, trans_id, lockup, cond);
  	bt_addrid(filenum, page, slot_num, RID, trans_id, lockup, cond)
  	bt_split_page(trans_id, filenum, leaf, new_page@, slot_num, length)
  	bt_traverse(filenum, key, leaf@, slot_num@, parent_index@, 
       parent_list@, trans_id, lockup, oper, cond);
    clean_locks(parent_list, parent_index, trans_id);
  
   EXPORTS:
  	st_insertindex(filenum, key, rid, trans_id, lockup, cond);
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>


st_insertindex(filenum, key, ridptr, trans_id, lockup, cond)
TWO	filenum;	    /* open file no of btree */
KEY	*key;	    /* ptr to key descriptor */
RID	*ridptr;	/* ptr to rid of record */
int     trans_id;
short   lockup;
short   cond;	    /* true if conditional locks are to be used */

/* This routine insert a new index into the given btree.
  	It splits a leaf page (into two leaf pages) if the page where
  	the key is found or supposed to be does not have enough room.
  
   RETURNS:
  	eNOERROR if successful
  
   SIDE EFFECTS:
  	The leaf page may be split, and the spliting may further propagate 
  	recursively upward.
  
   ERRORS:
  	None
*/
  
{
    PARENTINDEX	parent_index;	/* index for parent_list */
    PARENTLIST	parent_list;	/* hierarchical path of traversal */
    BTREEPAGE	*leaf;	    /* leaf page to insert key on */
    BTREEPAGE	*new_page;	/* pointer to the new leaf page */
    TWO	    slot_num;	/* position for new key */
    PID	    new_pid;	/* pid of the new leaf page */
    FID	    fileid;
    TWO     needdata;       /* bytes needed in page */
    TWO	    length;		/* length of new stuff to be added */
    TWO	    count;		/* # of RIDs on the list */
    int	    e;		/* for returned errors */
    char    buff[ENDDATA];	/* temp buffer to gather newentry */

#ifdef	TRACE
    if (checkset(&Trace2, tINTERFACE)) {
    	printf("st_insertindex(filenum=%d, key =", filenum);
    	PRINTKEY(key);
    	printf(", rid ="); PRINTRIDPTR(ridptr); printf(")\n");
    }
#endif

    CHECKOFN(filenum);
    CHECKWP(filenum);

    /* locate the position where the new index should go */
    e = bt_traverse(filenum,key, &leaf, &slot_num, &parent_index,
        parent_list, trans_id, lockup, BT_INSERT, cond);
    if (e != e2KEYNOTFOUND && e < eNOERROR) return(e); /* modified by PHJ */

    fileid = leaf->btcontrol.fileid;

    if (e == e2KEYNOTFOUND) {
    /* construct a brand new key-rid pair in local buffer */
    	MAKEKEYRID(buff,key->length,key->value,(char *)ridptr,length);
    	MAKEALIGN(length);
    	count = 0;
    }
    else 
    { 
	/* put the RID which is to be appended in local buffer */
    	if(leaf->btcontrol.unique)
    	{
    	    if (lockup) 
    	       clean_locks(parent_list, parent_index, trans_id);
    	    return(e2KEYALREADYEXISTS);
    	}
    	length = sizeof(RID);
	bcopy((char *)ridptr, buff, sizeof(RID));
    	MOVERIDCOUNT((char *)&count, RIDCOUNT(leaf, slot_num));
    }

    /* Check if there is enough room for the new stuff, if not, 
       split the leaf (recursively)
       note : if rid count < 0 then it is an overflow entry,
              append a RID to an overflow entry will have no effect
    	  on the current leaf page	    		 */

    needdata = length;
    MAKEALIGN(needdata);
    needdata += sizeof(leaf->slot[0]);
    if (leaf->btcontrol.numfree < needdata && count >= 0) 
    {
    	/* split the old page into two evenly distrubuted pages */

    	e = bt_split_page(trans_id, filenum, leaf, &new_page, slot_num, length);
    	if (e < eNOERROR) 
	{
    	    (void) bf_freebuf(filenum, &leaf->btcontrol.thispage, (PAGE *)leaf);
    	    if (lockup)
    	            clean_locks(parent_list, parent_index, trans_id);
    	    return(e);
    	}
    	F_NUMPAGES(filenum)++;
    	F_STATUS(filenum) = DIRTY;
    	new_pid = new_page->btcontrol.thispage;

    	/* determine which page the passed-in key should go on.  Since 
    	   the parent_list has the original left page on the top of it,
    	   if the key is to go on the new right page the top of the 
    	   parent_list will have to be replaced by the new page */

    	count = leaf->btcontrol.numoffsets;

    	if ((slot_num < count) || 
    	    (slot_num == count && length != sizeof(RID) &&
    	    FREESPACE(leaf) > FREESPACE(new_page)))
    	{	
	    /* new stuff should goes to the old page */
    	    (void) bf_freebuf(filenum, &new_page->btcontrol.thispage, 
		(PAGE *)new_page);
    	}
    	else 
	{	/* new stuff should go to the new page */
    	    (void) bf_freebuf(filenum, &leaf->btcontrol.thispage, (PAGE *)leaf);
    	    leaf = new_page;
    	    parent_list[parent_index].page_id = new_pid;
    	    slot_num -= count;
    	}

    }
    else 
    {
    	PIDCLEAR(new_pid);
    }

    /* set the leaf page dirty since we are going to write to it */

    if (length == sizeof(RID)) 
    { 	
	/* append the new rid to the an existing key */
    	e = bt_addrid(filenum, leaf, slot_num, (RID *)buff, trans_id, lockup,
		cond);
    	CHECKERROR(e);
    }
    else 
    { 
    	/* add a new key-rid pair */
    	(void) bt_insertentry(filenum, leaf, length, buff, slot_num);
    	F_CARD(filenum)++;
    	F_STATUS(filenum) = DIRTY;
    }
    (void) bf_freebuf(filenum, &leaf->btcontrol.thispage, (PAGE *)leaf);

    /* finally, the key/ptr pair of the new right leaf has 
        to be inserted into its parent */
    if (!TESTPIDCLEAR(new_pid)) 
    {
    	e = bf_readbuf(trans_id, filenum, FC_FILEID(filenum), &new_pid, 
		(PAGE **) &new_page);
    	CHECKERROR(e);
    	e = bt_insertkey(filenum, parent_list, &parent_index, 
    	    KEYLEN(new_page,0), KEYVALUE(new_page, 0), 
    	    &new_pid, trans_id, lockup, cond);
    	(void) bf_freebuf(filenum, &new_page->btcontrol.thispage, (PAGE *)new_page);
    	CHECKERROR(e);
    }

    if (lockup)
        clean_locks(parent_list, parent_index, trans_id);
    return(eNOERROR);

} /* st_insertindex */

