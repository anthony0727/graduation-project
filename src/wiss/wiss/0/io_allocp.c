
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
/* Module io_allocpages: allocate pages for a file 

   IMPORTS:
	VolDev[]
	IO_AllocExtent()
	IO_AllocPageInExt()

   EXPORTS:
	io_allocpages()		Allocates new pages for a file
*/

#include	<wiss.h>
#include	<io.h>


io_allocpages(FileID, NearPID, NumPages, PageIDs)
FID	*FileID;	/* file that is to get the extra pages */
PID	*NearPID;	/* Allocate as near this pid as possible */
int	NumPages;	/* # of pages to allocate */
PID	PageIDs[];	/* This is an array of PIDs */

/* Allocate "NumPages" pages and return their IDs in "PageIDs[]".
   If NearPID is specified, allocate pages close to it if possible.

   Returns:
	IDs of allocated pages in PageIDs[]

   Side Effects:
	additional extents allocated if necessary

   Errors:
	e0NULLFIDPTR - null FID pointer
	e0NULLPIDPTR - null PID pointer
	e0INVALIDFID - invalid FID
	e0FIDPIDNOTMATCH - volume ID inconsistent in PID and FID
	e0VOLNOTMOUNTED - volume not mounted

   Bugs:
	does not validate NearPID (too expensive to do)
*/
{
 	register i;		/* volume table index */
 	register count;		/* how many pages allocated */
	int	filenum;	/* file # */
	int	e;		/* for returns errors */
	TWO	VolID;		/* volume ID */
	TWO	Prev, Next;	/* extent #'s */
	TWO	*ExtLinks;	/* extent link array in the header */
	VOLFILE	*fd;		/* pointer to the file descriptor */

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
	{
		printf("io_allocpages(FID="); PRINTFIDPTR(FileID);
		printf(",NearPID="); PRINTPIDPTR(NearPID);
		printf(",#Pages=%d,PIDs=0x%x)\n", NumPages, PageIDs);
	}
#endif

	/* check input parameters */
	if (NumPages <= 0)
		return(eNOERROR);	/* nothing to do */
	if (FileID == NULL)
		return(e0NULLFIDPTR);	/* null FID pointer */
	filenum = FileID->Ffilenum;
	if (PageIDs == NULL)
		return(e0NULLPIDPTR);	/* null PID pointer */

	/* look up the volume */

	/* ERROR THIS LINE SHOULD NOT BE COMMENTED OUT*/
	VolID = FileID->Fvolid;

	for (i = 0; smPtr->VolDev[i].VDvolid != VolID;)
	{
	    if (++i >= MAXVOLS) 
            { 
             return(e0VOLNOTMOUNTED);
            }
	}
	SetLatch(&smPtr->VolDev[i].latch,procNum, NULL);

	/* check the file number */
	if (filenum<0 || filenum>=(smPtr->VolDev[i].VDheader[MHEADER])->VH.VHmaxfile)
               { 
		ReleaseLatch(&smPtr->VolDev[i].latch,procNum);
		return(e0INVALIDFID);	/* invalid file number */
               }

	if (NearPID != NULL)
	{ /* allocate near the specifiey page to preserve locality */

		if (VolID != NearPID->Pvolid)
               { 
			ReleaseLatch(&smPtr->VolDev[i].latch,procNum);
			return(e0FIDPIDNOTMATCH);
                }

		count = IO_AllocPageInExt(i, 
				NearPID->Ppage/smPtr->VolDev[i].VDextsize, 
		    		NumPages, PageIDs, 100);
		if ((NumPages -= count) <= 0)
                     {
			ReleaseLatch(&smPtr->VolDev[i].latch,procNum);
			return(eNOERROR);	/* done */
                      }

		PageIDs += count;
	}

	/* locate the addresses of the file descriptor and the extent links */
	fd = &( (smPtr->VolDev[i].VDheader[XFILEDESC]+FileID->Ffilenum/FDPERPAGE)->
		VF[FileID->Ffilenum%FDPERPAGE] );
	if (fd->VFeff < 0)
                {
			ReleaseLatch(&smPtr->VolDev[i].latch,procNum);
			return(e0FILENOTINUSE);	/* not a valid file */
                 }
	ExtLinks = (TWO *) (smPtr->VolDev[i].VDheader[EXTLINKS]);

	/* follow down the chain of extents to and allocate pages */
	for (Next = fd->VFextlist; Next >= 0; Next = ExtLinks[(Prev=Next)])
	{

		count = IO_AllocPageInExt(i, Next, NumPages, PageIDs,fd->VFeff);
		if ((NumPages -= count) <= 0)
                { 
			ReleaseLatch(&smPtr->VolDev[i].latch,procNum);
			return(eNOERROR);	/* done */
                }

		PageIDs += count;
	}

	/* more pages are needed, allocate new extents - one at a time */
	while (NumPages > 0) 
	{
		e = IO_AllocExtents(i, 1, &Next);
/*
		CHECKERROR(e);
*/
		CHECKERROR_RLATCH(e,&smPtr->VolDev[i].latch, procNum);
		ExtLinks[Prev] = Next;
		Prev = Next;
		count = IO_AllocPageInExt(i, Next, NumPages, PageIDs,fd->VFeff);
		PageIDs += count;
		NumPages -= count;
	}

	ExtLinks[Prev] = -1;	/* end of the new chain */
	setbit(&smPtr->VolDev[i].VDdirty, EXTLINKS);
	ReleaseLatch(&smPtr->VolDev[i].latch,procNum);
	return(eNOERROR);

} /* io_allocpages */
