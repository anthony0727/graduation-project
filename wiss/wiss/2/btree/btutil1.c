
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
/* Module : Btree Utility Module (1)
  	This module contains routines that deal with a B-tree page.
	The correctness of the all the given parameters is assumed. 

   IMPORTS:
  	int compare_key(key1, key2, keytype, keylength);
  	io_allocpages(fileid, nearpid, numpages, newpid)
  	bf_setdirty(filenum, pageid, pageptr)
  	bf_getbuf(transId, filenum, fid, pageid, returnpage)
  
   EXPORTS:
  	bt_compress_page(filenum, pageptr)
  	bt_binary_search(k_len, k_val, pageptr, slotno_ptr)
  	bt_allocpage(transId, filenum, newpidprt, returnpage,
  		  	 keytype, pagetype, treetype, unique)
  	bt_insertentry(filenum, page, length, entrybuf, slotnum)
  	bt_deleteentry(filenum, page, slotnum)
  
*/

#include	<wiss.h>
#include	<st.h>
#include <stdio.h>

bt_compress_page(filenum, btpage)
int		filenum;	/* file number */
register BTREEPAGE *btpage;	/* page to be compressed	*/

/* This routine copies a page into a local buffer, moving 
   only valid entries, so that holes in the page will be left behind.
   Offsets in the slot array are updated in place. 
   Finally, we copy the data back onto the original page.
  
   Returns:
	None
  
   Side Effects:
  	None

   Errors:
	None
*/
{
	register int	i;		/* the index, of course		*/
	register int	enddata;	/* show where data ends during copy */
	TWO		length;		/* length of the entry to copy */
	BTREEPAGE	scratch;	/* page buffer on program stack */

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("bt_compress_page(filenum=%d, btpage=0x%x)\n", 
			filenum, btpage);
#endif
#ifdef	DEBUG
	if (checkset(&Trace2, tCOMPRESS)) {
		printf("bt_compress_page: page to be compressed =\n");
		bt_print_btpage(btpage);
	}
#endif

	/* loop through the entries on btpage and copy them onto scratch.  */
	if (btpage->btcontrol.pagetype == ROOTPG) {
	    enddata =  (int) sizeof(KEYINFO);
	    movebytes(scratch.data, btpage->data, (int) sizeof(KEYINFO));
	}
	else enddata = 0;
	for(i = 0; i < btpage->btcontrol.numoffsets; i++) {
		if (btpage->slot[-i] == EMPTYSLOT) continue;
		/* get the length of entry i and copy it to buffer */
		ENTRYLEN(btpage, i, length);
		movebytes(&scratch.data[enddata], ENTADDR(btpage, i), length);
		btpage->slot[-i] = enddata;	/* adjust slot on old page */
		MAKEALIGN(length);
		enddata += length;
	}

	/* copy the compressed data from scratch back into btpage */
	movebytes((char *)btpage->data, (char *)scratch.data, enddata);
	btpage->btcontrol.enddata = enddata;	/* where the data area ends */
	btpage->btcontrol.numfree = USABLESPACE(btpage);

#ifdef	DEBUG
	if (checkset(&Trace2, tCOMPRESS)) {
		printf("bt_compress_page: page compressed =\n");
		bt_print_btpage(btpage);
	}
#endif

	(void) bf_setdirty(filenum, &btpage->btcontrol.thispage, btpage);
	return(eNOERROR);

} /* bt_compress_page */


bt_binary_search(k_len, k_val, page, slot_num)
TWO		k_len;		/* length of the given key 	*/
char		*k_val;		/* key to be searched for	*/
BTREEPAGE	*page;		/* page the search is done in	*/
TWO		*slot_num;	/* returned slot number	@	*/

/* This routine searches a page for a given key.  If the key is found,
   the slot number of the key on the page is returned via slot_num.
   Otherwise, the slot number where the key should have been found 
   (assuming sequential order of keys) is passed back and error code 
   e2KEYNOTFOUND is returned.
  
   RETURNS:
  	slot number via parameter slot_num
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	e2KEYNOTFOUND if search fails
*/
{
	int		e;	/* for returned errors */
	enum data_type	keytype;/* which type of keys */
	int	low, high;	/* limits and mid point for the	*/
	register int	mid;	/* binary search		*/
	TWO		len;	/* length of the current key on the page */
	register	result;	/* result of key comparison	*/

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL)) {
		printf("bt_binary_search(search key = ");
		print_data(page->btcontrol.keytype, k_len, k_val);
		printf(", *page=0x%x, *slot_num=0x%x)\n", page, slot_num);
	}
#endif
#ifdef	DEBUG
	if (checkset(&Trace2, tSEARCH)) {	
		printf("bt_binary_search: page to be searched =\n");
		bt_print_btpage(page);
	}
#endif
	e = e2KEYNOTFOUND;
	keytype = (enum data_type) page->btcontrol.keytype;
	for (low=mid=0, high = page->btcontrol.numoffsets-1; low <= high;) {
		mid = (low + high)/2;
		if (keytype == TSTRING) { /* key length may be variable */
			len = KEYLEN(page, mid); /* length of key on page */
			result = compare_key(k_val, KEYVALUE(page, mid), 
		     		keytype, MIN(len, k_len));
			if (!result) result=k_len-len; /* longer wins */
		}
		else result=compare_key(k_val,KEYVALUE(page,mid),keytype,k_len);

		/* stop the loop if key found, otherwise adjust bounds */
		if (result == 0) { /* key found */
			*slot_num = mid;
			e = eNOERROR;
			break;
		}
		if (result > 0) low = mid + 1;	/* page_key < key */
		else high = mid - 1;	 	/* page_key > key */

	} /* end of binary search */
	
	if (low > high) /* all keys are smaller than the given key */
		*slot_num = (low > mid) ? low : mid;

#ifdef	DEBUG
	if (checkset(&Trace2, tSEARCH))
		printf("bt_binary_search: slotnum = %d, on %s\n", 
			*slot_num, e==eNOERROR?"success":"failure");
#endif

	return(e);	/* eNOERROR or e2KEYNOTFOUND */

} /* bt_binary_search */


bt_allocpage(transId, filenum, returnpid,returnpage, keytype, pagetype, treetype, unique)
int		transId;	/* transaction identifier */
int		filenum;	/* file number */
PID		*returnpid;	/* pointer to pid of newly allocated page */
BTREEPAGE	**returnpage;	/* buffer pointer of the new page */
ONE		keytype;	/* key type */
ONE		pagetype,	/* ROOTPG, NODEPG, LEAFPG */
		treetype,	/* INDEX, LINK?, VERSION? */
		unique;		/* primary or secondary key */

/* This routine allocates a new B-tree page and initializes its control info
  
   Returns:
  	the ID and a buffer for the newly allocated page are returned
  
   Side Effects:
	the buffer containing the new page is fixed

   Errors:
	None 
  
   Bugs:
  	This routine does not link the new page up to its previous
  	and/or following pages.  
	Header info on the root page is not initialized.
*/
{
	int		e;		/* error code */
	BTCONTROL	*p;		/* ptr to beginning of control info */

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL)) {
		printf("bt_allocpage(filenum=%d", filenum);
		printf(", pidptr=0x%x, pageptr=0x%x, ", returnpid, returnpage);
		printf("keytype=%d, pagetype=%c, treetype=%c, unique=%s)\n",
			(int)keytype, pagetype, treetype, unique ? "YES":"NO");
	}
#endif

	/* allocate a new page and get a buffer for it */
	e = io_allocpages(&(F_FILEID(filenum)), (PID *)NULL, 1, returnpid);
	CHECKERROR(e);
	e = bf_getbuf(transId, filenum, FC_FILEID(filenum), returnpid, 
		(PAGE **)returnpage);
	CHECKERROR(e);

	/* initialize common control info */
	p = &((*returnpage)->btcontrol);
	p->startdata = (pagetype != ROOTPG) ? 0 : (int)sizeof(INDEXHEADER);
	p->enddata = p->startdata;
	p->numoffsets = 0;
	p->fileid = F_FILEID(filenum);
	p->thispage = *returnpid;
	p->numfree = USABLESPACE((*returnpage));
	p->keytype = keytype;
	p->pagetype = pagetype;
	p->treetype = treetype;
	p->unique = unique;
	p->next = p->prev = p->overflow = p->pid0 = NULLPAGE;

	return (eNOERROR);

} /* bt_allocpage */


bt_insertentry(filenum, btpage, len, entryptr, slotnum)
int		filenum;	/* file number */
register BTREEPAGE *btpage;	/* pointer to page to put entry on */
TWO		 len;		/* length of the new entry */
char		 *entryptr;	/* pointer to the entry to be inserted */
TWO		 slotnum;	/* where new entry goes */

/* Insert an entry (a key/ptr or a key/RID list) into a B-tree page.
  
   Returns:
  	None
  
   Side Effects:
  	None
  
   Errors:
	None

   Bugs:
   	assumes the page has enough room to for the new entry 
*/
{
	int	i;		/* loop index */
	TWO	*s;		/* pointer to slot array */

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
	  printf("bt_insertentry(filenum=%d,page=0x%x,len=%d,entry=0x%x,slot=%d)\n",
			filenum, btpage, len, entryptr, slotnum);
#endif

	MAKEALIGN(len);

	/* compress the page if there isn't enough room at the end */
	if (USABLESPACE(btpage) < len+(int)sizeof(btpage->slot[0]))
		(void) bt_compress_page(filenum, btpage);

	if (USABLESPACE(btpage) < len + (int)sizeof(btpage->slot[0])) 
	{
	  (void) fprintf(stderr,
	   "WISS internal error: entry too large to fit in page (%d)\n",
	      len+sizeof(btpage->slot[0]));
	   return(e2KEYLENGTHTOOLONG);
   	}

	/* make room in the slot array for the new entry */
	i = btpage->btcontrol.numoffsets;
	s = &btpage->slot[-i];
	if ((i -= slotnum) > 0) 
 	 movebytes((char *)s, (char *)(s+1), i*(int)sizeof(btpage->slot[0])); 

	/* append the entry to end of the data area and update control info */
	btpage->slot[-slotnum] = btpage->btcontrol.enddata;
	movebytes(&(btpage->data[btpage->btcontrol.enddata]), entryptr, len);
	btpage->btcontrol.enddata += len;
	btpage->btcontrol.numfree -= len+ (int)sizeof(btpage->slot[0]);
	btpage->btcontrol.numoffsets++;
	(void) bf_setdirty(filenum, &btpage->btcontrol.thispage, btpage);

#ifdef	DEBUG
	if (checkset(&Trace2, tINSERTENTRY)) {
		printf("bt_insertentry: after inserting at %d\n", slotnum);
		bt_print_btpage(btpage);
	}
#endif

	return(eNOERROR);

} /* bt_insertentry */


bt_deleteentry(filenum, btpage, slotnum)
int		filenum;	/* file number */
BTREEPAGE	 *btpage;	/* pointer to page where entry is on */
TWO		 slotnum;	/* which entry to delete */

/* Delete an entry (a key/ptr or a key/RID list) from a B-tree page.
  
   REturns:
  	None
  
   Side Effects:
  	None
  
   Errors:
  	None
*/
{
	TWO	i;		/* loop index */
	TWO	len;		/* length of the entry to be deleted */
	TWO	*s;		/* slot array pointer */

#ifdef	TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("bt_deleteentry(filenum=%d,page=0x%x,slot=%d)\n", 
			filenum, btpage, slotnum);
#endif

	/* get the length of the entry */
	ENTRYLEN(btpage, slotnum, len);
	MAKEALIGN(len);

	/* update the control info and close the gap in the slot array */
	btpage->btcontrol.numfree += len+(int)sizeof(btpage->slot[0]);
	btpage->btcontrol.numoffsets--;
	i = btpage->btcontrol.numoffsets;
	s = &btpage->slot[-i];
	if ((i -= slotnum) > 0) 
		movebytes((char *)(s+1),(char *)s,i*sizeof(btpage->slot[0])); 
	(void) bf_setdirty(filenum, &btpage->btcontrol.thispage, btpage);

#ifdef	DEBUG
	if (checkset(&Trace2, tINSERTENTRY)) {
		printf("bt_deleteentry: after deleting entry %d\n", slotnum);
		bt_print_btpage(btpage);
	}
#endif

	return(eNOERROR);

} /* bt_deleteentry */

