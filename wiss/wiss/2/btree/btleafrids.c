
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
/* Module btleafrids
  	This module implements an abstract data type, RID list, that exists
  	on B-tree leaf pages. Operations on RID list includes 
  	fetching, inserting and deleting a particular RID.
  	The address of a RID in a RID list is represented by a triplet
  	(page, slot #, offset), where (page, slot#) specifies where the entire
  	key/RID-list entry is located and offset is the index into the RID list.
  	An overflow RID list is stored in a long data item. In this case,
  	the RID of the long data item and the negated RID count are stored
  	where a normal RID list would be. MAXRIDCOUNT is a constant in st_bt.h
  	that set the upper limit for the length of a normal RID list.
  
   IMPORTS:
	bf_setdirty(filenum, pageid, pageptr)
  	bt_compress_page(filenum, page)
  	st_createlong(filenum, dirRID, trans_id, lockup, cond)
  	st_destroylong(filenum, dirRID, trans_id, lockup, cond)
  	st_readframe(filenum, dirRID, offset, Recadr, length, 
		trans_id, lockup, cond)
  	st_insertframe(filenum, dirRID, offset, recadr, length, 
		trans_id, lockup, cond)
  	st_deleteframe(filenum, dirRID, offset, length, 
		trans_id, lockup, cond)
  
   EXPORTS:
  	bt_getrid(filenum, leaf_page, slot_num, offset, ridptr@, 
		trans_id, lockup, cond)
  	bt_addrid(filenum, leaf_page, slot_num, ridptr, trans_id, lockup, cond)
  	bt_zaprid(filenum, leaf_page, slot_num, ridptr, trans_id, lockup, cond)
*/
  
#include <wiss.h>
#include <st.h>

bt_getrid(filenum, leaf, slotnum, offset, ridptr, trans_id, lockup, cond)
int		filenum;	/* open file nubmer of the index file */
BTREEPAGE	*leaf;		/* which page the entry is on */
TWO		slotnum;	/* which slot on the page */
TWO		offset;		/* which RID on the list */
RID		*ridptr;	/* where to return the RID */
int             trans_id;
short           lockup;
short	        cond;  	    /* whether lock is to be conditional */

/* Retrieve a RID from a given entry on a leaf page
  
    Returns:
  	the RID addressed by (leaf, slotnum, offset)
  					
    Errors:
  	None
*/
{
	int		e = eNOERROR;	/* for returned errors */
	TWO		rid_count;	/* # of rids on the RID list */
	RID		dirrid;		/* directory of a long item */

#ifdef TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("bt_getrid(*leaf=0x%x, slot=%d, offset=%d, *rid=0x%x)\n",
			leaf, slotnum, offset, ridptr);
#endif

	/* get the # of RIDs on the list */
	MOVERIDCOUNT(&rid_count, RIDCOUNT(leaf, slotnum));

	/* retrieve the RID */
	if (rid_count < 0) { /* an overflow entry */
		MOVERID(&dirrid, RIDLIST(leaf, slotnum));
		e = st_readframe(filenum, &dirrid, 
			(int)(offset*(int) sizeof(RID)), (char *)ridptr, 
			(int)sizeof(RID), trans_id, lockup, cond);
	}
	else MOVERID(ridptr, RIDLIST(leaf, slotnum)+offset*((int)sizeof(RID)));

#ifdef TRACE
	if (checkset(&Trace2, tBTREEUTIL)) {
		printf("returning from bt_getrid with RID = ");
		PRINTRIDPTR(ridptr); printf("\n");
	}
#endif
	return(e);

} /* bt_getrid */


bt_addrid(filenum, leaf, slotnum, ridptr, trans_id, lockup, cond)
int		filenum;	/* open file # of the B-tree */
BTREEPAGE	*leaf;		/* which page the entry is on */
TWO		slotnum;	/* which slot on the page */
RID		*ridptr;	/* RID to be inserted */
int             trans_id;
short           lockup;
short	        cond;  	    /* whether lock is to be conditional */

/* Add a RID to an given entry on a page.
   If the RID list grows too long, it is changed into a long data item.
  
    Returns:
  	None
  					
    Side effects:
  	A long data item may be created as a result of RID list overflow
  
    Errors:
  	None
*/
{

	int		e = eNOERROR;	/* for returned errors */
	RID		dirrid;		/* directory of a long item */
	TWO		rid_count;	/* # of RIDs on the list of this key */
	TWO		length;		/* length of an entry */
	TWO             aligned;        /* aligned length of an entry */
	TWO             oldenddata;     /* enddata before adjustment */
	char		*entryptr;	/* pointer to a entry */
	char		buff[MAXKEYLEN + MAXRIDCOUNT *sizeof(RID)];


#ifdef TRACE
	if (checkset(&Trace2, tBTREEUTIL)) {
		printf("bt_addrid(ofn=%d, leaf=0x%x, slot=%d, RID=",
			filenum, leaf, slotnum);
		PRINTRIDPTR(ridptr); printf(")\n");
	}
#endif

	/* get the # of RIDs on the RID list */
	MOVERIDCOUNT(&rid_count, RIDCOUNT(leaf, slotnum));

	/* if the current RID list is not an overflow entry, but it has
	   grown too long, then convert it into a long data item  */

	if (rid_count >= MAXRIDCOUNT) {
		e = st_createlong(filenum, &dirrid, trans_id, lockup, cond);
		CHECKERROR(e);
		e = st_insertframe(filenum, &dirrid, 0, 
			(char *)RIDLIST(leaf, slotnum), rid_count*(int)sizeof(RID),
			trans_id, lockup, cond);
		CHECKERROR(e);
		leaf->btcontrol.numfree += (rid_count - 1) * (int)sizeof(RID);
		MOVERID(RIDLIST(leaf, slotnum), &dirrid);
		rid_count = (-rid_count);	/* encode the RID count */
		MOVERIDCOUNT(RIDCOUNT(leaf, slotnum), &rid_count);
	}

	/* append the new RID to the list */
	if (rid_count > 0) { /* a normal entry */
		/* calculate the length and address of the old entry */
		KEYRIDLEN(leaf, slotnum, length);
		aligned = length;
		MAKEALIGN(aligned);
		entryptr = ENTADDR(leaf, slotnum);

		/* Check if the space after enddata is enough; if not, 
		   move the old entry from the page to a local buffer 
		   and then compress the page 
		*/
		if (aligned + (int)sizeof(RID) > USABLESPACE(leaf)) {	

			/*
			 * the space after enddata is not enough;
			 * move the old entry from the page to a local buffer,
			 * compress the page, then move it back and adjust
			 * the page control info.
			 */
			movebytes(buff, entryptr, length);
			leaf->slot[-slotnum] = EMPTYSLOT; /* mark as unused */

			e = bt_compress_page(filenum, leaf);
			CHECKERROR(e);

			movebytes(&(leaf->data[leaf->btcontrol.enddata]),
			    buff, length);
			leaf->slot[-slotnum] = leaf->btcontrol.enddata;
 			leaf->btcontrol.enddata += aligned;

			leaf->btcontrol.numfree = USABLESPACE(leaf);

		} else if (leaf->slot[-slotnum]+aligned+(int)sizeof(int) < 
		   leaf->btcontrol.enddata) {

			/*
			 * move the old entry to enddata if it is not
			 * the last entry on this page
			 */
			movebytes(&(leaf->data[leaf->btcontrol.enddata]),
				entryptr, length);
			leaf->slot[-slotnum] = leaf->btcontrol.enddata;
 			leaf->btcontrol.enddata += aligned;
		}

		/* By now, the old entry has become the last (physically) 
		   entry on this page, so we simply append the rid to the 
	    	   very end of the data area 
		*/
		MOVERID(ENTADDR(leaf, slotnum) + length, ridptr);
		rid_count++;	/* increment and update the rid count on page */
		MOVERIDCOUNT(RIDCOUNT(leaf,slotnum), &rid_count);
		/*
		 * Adjust enddata, then updata numfree with the delta
		 */
		oldenddata = leaf->btcontrol.enddata;
		leaf->btcontrol.enddata = 
			leaf->slot[-slotnum]+length+(int)sizeof(RID);
		MAKEALIGN(leaf->btcontrol.enddata);
		leaf->btcontrol.numfree -=
		    (leaf->btcontrol.enddata - oldenddata);
	}
	else { /* an overflow RID list */
		MOVERID(&dirrid, RIDLIST(leaf, slotnum));
		e = st_insertframe(filenum, &dirrid,
			(-rid_count)*(int)sizeof(RID), (char *)ridptr, (int)sizeof(RID),
			trans_id, lockup, cond);
		CHECKERROR(e);
		rid_count--; /* "increment" the rid count */
		MOVERIDCOUNT(RIDCOUNT(leaf,slotnum), &rid_count);
	}
#ifdef	DEBUG
		if (checkset(&Trace2, tAPPENDRID)) {
			printf(" after inserting the new RID :\n");
			if (rid_count > 0) bt_print_btpage(leaf);
			else bt_print_overflow(filenum, &dirrid);
		}
#endif

	(void) bf_setdirty(filenum, &(leaf->btcontrol.thispage), leaf);

#ifdef TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("returning from bt_addrid : new RID count = %d\n",
			(rid_count < 0) ? (-rid_count) : rid_count);
#endif

	return(eNOERROR);

} /* bt_addrid */


bt_zaprid(filenum, leaf, slotnum, ridptr, trans_id, lockup, cond)
int		filenum;	/* open file # of the B-tree */
BTREEPAGE	*leaf;		/* which page the entry is on */
TWO		slotnum;	/* which slot on the page */
RID		*ridptr;	/* which RID on the list */
int             trans_id;
short           lockup;
short	        cond;  	    /* whether lock is to be conditional */

/* Delete a RID from a list on a given page.
   If RID list becomes empty after the deletion, remove the entire entry.
  
    Returns:
  	None
  					
    Side effects:
  	A entry may be deleted if its RID list becomes empty
  
    Errors:
  	e2NORIDMATCH : the given RID is not found
*/
{
	int		e = eNOERROR;
	RID		dirrid;		/* directory of a long item */
	RID		trid;		/* temporary RID for comparison */
	TWO		rid_count;	/* # of RIDs on the list of this key */
	TWO		i;		/* loop index */
	TWO		offset;		/* offset into the RID list */
	TWO		entrylength;	/* length of the whole entry */
	RID		rid;		/* RID to be search for */
	register RID	*rptr;

#ifdef TRACE
	if (checkset(&Trace2, tBTREEUTIL)) {
		printf("bt_zaprid(filenum=%d, leaf=0x%x, slot=%d, RID=",
			filenum, leaf, slotnum);
		PRINTRIDPTR(ridptr); printf(")\n");
	}
#endif

	/* copy the given RID into a local variable */
	rid = *ridptr;

	MOVERIDCOUNT(&rid_count, RIDCOUNT(leaf, slotnum));
	if (rid_count < 0) { /* an overflow entry */
		MOVERID(&dirrid, RIDLIST(leaf,slotnum));
		for (i = 0; i < (-rid_count); i++) {
		    e = st_readframe(filenum, &dirrid, i*(int)sizeof(RID), 
		    (char *)&trid, (int) sizeof(RID), trans_id, lockup, cond);
		    CHECKERROR(e);
		    if (RIDEQ(trid, rid)) break;
		}
		if (i == (-rid_count)) return(e2NORIDMATCH);
		else offset = i;

		if( (++rid_count) == 0) { /* delete the whole entry */
			ENTRYLEN(leaf, slotnum, entrylength);
			e = st_destroylong(filenum, &dirrid, trans_id, lockup,
				cond);
			CHECKERROR(e);
			for (i = slotnum; i < leaf->btcontrol.numoffsets-1; i++)
				leaf->slot[-i] = leaf->slot[-(i+1)];
			leaf->btcontrol.numoffsets--;
			MAKEALIGN(entrylength);
			leaf->btcontrol.numfree += entrylength
				 + (int)sizeof(leaf->slot[0]);
			F_CARD(filenum)--, F_STATUS(filenum) = DIRTY;
		}
		else {
			e = st_deleteframe(filenum,&dirrid,
				offset*((int)sizeof(RID)), (int)sizeof(RID), 
				trans_id, lockup, cond);
			CHECKERROR(e);
			MOVERIDCOUNT(RIDCOUNT(leaf, slotnum), &rid_count);
		}
	}
	else  { /* a normal entry */
		rptr = (RID *) (RIDLIST(leaf, slotnum));
		for (i = 0; i < rid_count; i++, rptr++) {
			MOVERID(&trid, rptr); 
			if (RIDEQ(trid, rid)) break;
		}
		if (i == rid_count) return(e2NORIDMATCH);
		if (--rid_count != 0) { /* still more RIDs left on the list */
			MOVERIDCOUNT(RIDCOUNT(leaf,slotnum),&rid_count)
			MOVERID(rptr, (rptr + (rid_count - i)));
			leaf->btcontrol.numfree += (int) sizeof(RID);
		}
		else { /* no more rid with this key */
			/* delete the whole entry by adjusting the slot array */
			ENTRYLEN(leaf, slotnum, entrylength);
			for (i = slotnum; i < leaf->btcontrol.numoffsets-1; i++)
				leaf->slot[-i] = leaf->slot[-(i+1)];
			leaf->btcontrol.numoffsets--;
			MAKEALIGN(entrylength);
			leaf->btcontrol.numfree += entrylength
				+ (int) sizeof(leaf->slot[0]);
			F_CARD(filenum)--, F_STATUS(filenum) = DIRTY;
		}
	}

	(void) bf_setdirty(filenum, &(leaf->btcontrol.thispage), leaf);

#ifdef TRACE
	if (checkset(&Trace2, tBTREEUTIL))
		printf("returning from bt_zaprid\n");
	if (checkset(&Trace2, tTREEDUMP)) bt_print_btpage(leaf);
#endif

	return(eNOERROR);

} /* bt_zaprid */

