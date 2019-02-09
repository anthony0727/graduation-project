
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
/* Module r_slide : move the contents of a page around 

   IMPORTS:
	None

   EXPORTS:
	r_slide(page, slot, newlength, recptr, trans_id, lockup, cond)
*/

#include	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>


r_slide (Page, SlotNumber, NewLength, RecPtr, trans_id, lockup, cond)
register DATAPAGE	*Page;		/* buffer containing data page */
register int		SlotNumber;	/* which slot to change */
int			NewLength;	/* length to make slot's data area */
RECORD			**RecPtr;	/* out parameter - record address */
int                     trans_id;       
short                   lockup;        
short			cond;

/* Slide around the information on a data page and update slots, making room
   	for a record of the specified length, NewLength.
   	if (NewLength == REMOVEREC) remove the record entirely
   	else NewLength is size of data part--record header is added in here

   Returns:
	Pointer to start of record (via RecPtr)

   Side Effects:
	None

   Errors generated here:
	e2BADSLOTNUMBER		an invalid slot number
	e2NOROOMONPAGE		no more room on this page for expansion

   Crocks:
	Assumes that all record headers take up a multiple of ALIGN bytes.

   Special note:
	This routine worries about alignment of records within the page, and
	may modify the actual amount of space the record needs, accordingly.

*/
{
	register int	s;		/* slot indexer */
	RECORD		*rec;		/* start of record */
	int		Length;		/* temporary length variable */
	int		freelength;	/* size of free area on page */
	int		expand;		/* amount to expand page */
	int		oldrecord;	/* does this record already exist? */
	int		e;

#ifdef TRACE
	if (checkset (&Trace2, tSLIDE))
		printf("r_slide (Page=0x%x, SlotNumber=%d, NewLength=%d)\n",
			Page, SlotNumber, NewLength);
#endif
	/* check the slot number */
	if (SlotNumber < 0 || SlotNumber >= Page->ridcnt)
		return(e2BADSLOTNUMBER);

	/* lock the page in exclusive mode to avoid conflicts */
	if (lockup)
	{
	    e = lock_page (trans_id, Page->fileid, Page->thispage, l_X, COMMIT, 
		cond);
	    CHECKERROR(e);
	}

	/* calculate amount of free data space on page */
	freelength = DATAFREE(Page);

	/* Is this an existing record ? */
	oldrecord = Page->slot[-SlotNumber] != EMPTYSLOT;

	/* claculate how much to expand (or shrink) */
	if (oldrecord) {
		rec = (RECORD *) &(Page->data[ Page->slot[-SlotNumber] ]);
		Length = rec->length;
		MAKEALIGN(Length);
		if (NewLength == REMOVEREC) expand = -(Length + HEADERLEN);
		else expand = NewLength - Length;
	}
	else {	/* brand new record */
		if (NewLength == REMOVEREC)	/* can't empty an unused slot */
			return (e2BADSLOTNUMBER);
		expand = NewLength + HEADERLEN;	/* how much space we want */
	}
	MAKEALIGN(expand);

	/* at this point, we have
		expand = (signed) amount of extra room to make
		rec    = pointer to start of record if it already exists
	*/

	if (freelength < expand)		/* can we supply enough room? */
		return (e2NOROOMONPAGE);

	if (oldrecord) {
		char	*firstmove;	/* first byte to move */
		int	nbytes;		/* number of bytes to be moved */

		/* move all slots that get in the way */
		for (s = 0; s < Page->ridcnt; s++)	
			if (Page->slot[-s] > Page->slot[-SlotNumber])
				Page->slot[-s] += expand; /* adjust offsets */
		firstmove = rec->data + Length;	/* point to next record */
		nbytes = Page->free - Page->slot[-SlotNumber]-Length-HEADERLEN;
		movebytes(firstmove + expand, firstmove, nbytes);
		Page->free += expand;		/* start of next aligned area */

		if (NewLength == REMOVEREC) {
			Page->slot[-SlotNumber] = EMPTYSLOT; 
			/* remove empty trailing slots */
			for (s = Page->ridcnt-1; 
				s >= 0 && Page->slot[-s]==EMPTYSLOT; s--)
					(Page->ridcnt)--; /* one less slot */
			rec = NULL;			/* no record now! */
		}
	}
	else {	/* setting up the structure for a new record */
		rec = (RECORD *) &(Page->data[Page->free]);
		Page->slot[-SlotNumber] = Page->free;	/* offset... */
		Page->free += expand;		/* start of next aligned area */
	}

	if (rec != NULL) rec->length = NewLength;
	*RecPtr = rec;	/* return the starting address of the record */

	return(eNOERROR);

} /* r_slide */

