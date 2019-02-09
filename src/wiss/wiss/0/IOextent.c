
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


/* Module IOextent: Extent map and page maps manipulation

   IMPORTS:
	VolDev[]
	nextset()
	checkset()
	setbit()
	clearbit()
	wsetmap()

   EXPORTS:
	IO_AllocExtents()	Allocates extents from the free extent pool
	IO_FreeExtents()	Frees extents back to the free extent pool
	IO_AllocPageInExt()	Allocates pages in a given extent
	IO_FreePageInExt()	Frees pages in a given extent

   NOTES:
	IO_AllocExtents and IO_FreeExtents deal with extents, and
	are responsible for maintaining the "extent map".
	IO_AllocPageInExt and IO_FreePageInExt deal with pages within
	an extent, and are responsible for maintaining the "page maps".
*/

#include	<wiss.h>
#include        <io.h>


IO_AllocExtents(Vix, NumExt, Exts)
int	Vix;		/* volume table index */
int     NumExt;		/* # of extents needed */
TWO	Exts[];		/* where to return the extents */

/* Allocate "NumExt" free extents, contiguous if possible.

   Returns:
	indices of the allocated extents in Exts[]

   Side Effects:
	extent bit map updated

   Errors:
        e0NOSPACEONDISK - # of extents asked > # of extents available.
*/
{
	int		lwm = (smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHextlwm - 1;
	ONE		*emap = (smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHmap;
	int		emapsize = smPtr->VolDev[Vix].VDnumext; /* extent map size */
	int		first;	/* start of consecutive extents */
	register int	next;	/* next extent in the cluster */
	register int	count;	/* how many extents found so far */
	PID		PageID;	/* for resetting page maps */

#ifdef TRACE
	if (checkset(&Trace0, tALLOCATION))
		printf("IO_AllocExtents(Vix=%d,NumExt=%d,Exts=0x%x)\n",
			Vix, NumExt, Exts);
#endif


	for(next = lwm, count = 0; count < NumExt;) 
	{
		/* start from where we left out last time and
		    look for "NumExt" consecutive extents */

		if ( (first = nextset(emap, emapsize, next)) < 0)
			next = emapsize+1; 	/* no consecutive extents */
		else for (next = first+1, count = 1; count < NumExt &&
			checkset(emap,next) && next < emapsize; next++,count++);

		if (count == NumExt) /* contiguous extents found */
			for (next = 0; next < count; next++)
				Exts[next] = first + next;
		else if (next >= emapsize) /* return whatever is available */
			for (next = lwm, count = 0; count < NumExt; 
					Exts[count++] = next)
				if ( (next = nextset(emap,emapsize,next)) < 0)
					return(e0NOSPACEONDISK);

	} /* end for */

	/* update allocation info and mark the main header & extent map dirty */
	for (count = 0; count < NumExt; count++)
		clearbit(emap, Exts[count]);	/* mark extent allocated */
	setbit(&smPtr->VolDev[Vix].VDdirty, EXTMAP);
	lwm = nextset(emap, emapsize, lwm);	/* the new low water mark */
	(smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHextlwm = (lwm < 0) ? 0 : lwm;
	(smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHfreeext -= NumExt;
	setbit(&smPtr->VolDev[Vix].VDdirty, MHEADER);

	/* mark all the pages in each allocated extent free */
	PageID.Pvolid = smPtr->VolDev[Vix].VDvolid;
	for (count = 0; count < NumExt; count++)
	{
		PageID.Ppage = Exts[count] * smPtr->VolDev[Vix].VDextsize;
		(void) IO_FreePageInExt(Vix, -1, &PageID);
	}

	return(eNOERROR);
	
} /* IO_AllocExtents */


IO_FreeExtents(Vix, NumExt, Exts)
int	Vix;		/* Volume table index */
int     NumExt;		/* # of extents to be released */
TWO	Exts[];		/* extents to be released */

/* Release "NumExt" extents back to the free extent pool.

   Returns:
	None

   Side Effects:
	extent bit map updated

   Errors:
	None
*/
{
	register 	i;
	PID             PageID;
	int		lwm = (smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHextlwm;
	ONE		*emap = (smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHmap;

#ifdef TRACE
	if (checkset(&Trace0, tALLOCATION))
		printf("IO_FreeExtents(Vix=%d,NumExt=%d,Exts=0x%x)\n",
			Vix, NumExt, Exts);
#endif

	PageID.Pvolid = smPtr->VolDev[Vix].VDvolid;
	/* update extent map and adjust the low water mark if necessary */
	for (i = 0; i < NumExt; i++)
	{
	    PageID.Ppage = Exts[i] * smPtr->VolDev[Vix].VDextsize;
/*
	    if (lockup) {
	        m_release_page(trans_cell, PageID);
	    }
*/
	    setbit(emap, Exts[i]);		/* mark the extent free */
	    if (Exts[i] < lwm) lwm = Exts[i];
	}

	setbit(&smPtr->VolDev[Vix].VDdirty, EXTMAP);

	/* update control info in header, and mark the header dirty */
	(smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHextlwm = lwm;
	(smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHfreeext += NumExt;
	setbit(&smPtr->VolDev[Vix].VDdirty, MHEADER);

	return(eNOERROR);
	
} /* IO_FreeExtents */


int IO_AllocPageInExt(Vix, ExtNum, NumPages, PageIDs, FillFactor)
int	Vix;		/* volume table index */
int	ExtNum;		/* which extent to allocate */
int	NumPages;	/* how many pages to allocate */
PID	PageIDs[];	/* where to return the IDs of the pages allocated */
int	FillFactor;	/* how full should we allow an extent to be (in %) */

/* This routine allocates (subject to FillFactor) up to NumPages from 
   an extent and returns their page numbers in PageIDs[];

   Returns:
	number of pages allocated from extent "ExtNum"
	IDs of the allocated pages in PageIDs[]

   Side Effects:
	page map for the extent may be updated

   Errors:
	None
*/
{
	register i, j;		/* volatile registers */
	ONE	*pmap;		/* page map for this extent */
	int	VolID;		/* volume ID */
	int	page_no;	/* starting page # of the extent */
	int	extsize = smPtr->VolDev[Vix].VDextsize; /* size of an extent */

#ifdef TRACE
	if (checkset(&Trace0, tALLOCATION))
	{
		printf("IO_AllocPageInExt(Vix=%d,ExtNum=%d,NumPages=%d,",
			Vix, ExtNum, NumPages);
		printf("PageIDs=0x%x,EFF=%d)\n", PageIDs, FillFactor);
	}
#endif

	/* locate the page map of the extent */
	j = (extsize-1) / BITSPERBYTE + 1;  /* size of a page map (in bytes) */
	i = PAGESIZE / j;  		    /* # of page maps per block */
	pmap = &( (smPtr->VolDev[Vix].VDheader[PAGEMAP]+ExtNum/i)->CH[ExtNum%i*j] ); 
	
	/* calculate how many pages to allocated from this extent */
	i = countmap(pmap, extsize);	/* how many are free in the extent */
	if (FillFactor != 100)	/* reserve some pages for future */
		i -= extsize * (100 - FillFactor) / 100;  
	if (i <= 0)
		return(0);	/* sorry! none to spare */
	if (i < NumPages)
		NumPages = i;	/* can only spare this much */

	/* mark pages allocated & the page map dirty */
	page_no = extsize * ExtNum;
	VolID = smPtr->VolDev[Vix].VDvolid;
	for(i = 0, j = -1; i < NumPages; i++, PageIDs++)
	{
		j = nextset(pmap, extsize, j);	/* the next available page */
		PageIDs->Ppage = page_no + j;	/* synthesize its PID */
		PageIDs->Pvolid = VolID;
		clearbit(pmap, j);	/* mark the page allocated */
	}
	setbit(&smPtr->VolDev[Vix].VDdirty, PAGEMAP);

	return(NumPages);	/* return the actual # of pages allocated */

} /* IO_AllocPageInExt */


IO_FreePageInExt(Vix, NumPages, PageID)
int	Vix;		/* volume table index */
int	NumPages;	/* # of pages to free */
PID	*PageID;	/* which page to start */

/* Free "NumPages" Pages starting from pageID (within the same extent).
   If NumPages is negative, then free all the pages in the extent.

   Returns:
	None
	
   Side Effects:
	page map for the extent updated 

   Errors:
	e0INVALIDPID - invalid page ID
*/
{
	register i, j;		/* volatile registers */
	register ExtNum;	/* index of the extent */
	ONE	*pmap;		/* address of the page map */
	int	extsize = smPtr->VolDev[Vix].VDextsize; /* size of an extent */

#ifdef TRACE
	if (checkset(&Trace0, tALLOCATION))
		printf("IO_FreePageInExt(Vix=%d,NumPages=%d,PageID=0x%x)\n",
			Vix, NumPages, PageID);
#endif

	/* locate the page map of the extent */
	ExtNum = PageID->Ppage / extsize;
	if (ExtNum < 0 || ExtNum >= smPtr->VolDev[Vix].VDnumext)
	{
#ifdef DEBUG
	if (checkset(&Trace0, tALLOCATION))
		printf("Invalid page number %d detected in IO_FreePageInExt\n",
			PageID->Ppage);
#endif
		return(e0INVALIDPID);	/* invalid page ID */
	}
	if (checkset( (smPtr->VolDev[Vix].VDheader[MHEADER])->VH.VHmap, ExtNum ))
	{
#ifdef	DEBUG
	if (checkset(&Trace0, tALLOCATION))
		printf("Attempt to free an unused page %d (IO_FreePageInExt)\n",
			PageID->Ppage);
#endif
		return(e0INVALIDPID);	/* invalid page ID */
	}
	j = (extsize-1) / BITSPERBYTE + 1;  /* size of a page map (in bytes) */
	i = PAGESIZE / j;  		    /* # of page maps per block */
	pmap = &( (smPtr->VolDev[Vix].VDheader[PAGEMAP]+ExtNum/i)->CH[ExtNum%i*j] ); 

	if (NumPages < 0) 	/* free all pages in the extent */
		wsetmap(pmap, extsize);
	else
	{
		j = PageID->Ppage % extsize;	/* page # within the extent */
		i = (j + NumPages > extsize) ? extsize : j + NumPages;
		for (; j < i; j++)	/* free pages from j to i - 1 */
			if (checkset(pmap, j))
			{
#ifdef	DEBUG
	if (checkset(&Trace0, tALLOCATION))
		printf("Attempt to free an unused page %d (IO_FreePageInExt)\n",
			PageID->Ppage);
#endif
				return(e0INVALIDPID);	/* page not is use */
			}
			else setbit(pmap, j);	/* mark page free */
	}

	/* mark the page map dirty */
	setbit(&smPtr->VolDev[Vix].VDdirty, PAGEMAP);

	return(eNOERROR);

} /* IO_FreePageInExt */

