
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
/* Module io_filedir: maintain FileDir location for level 2 

   IMPORTS:
	VolDev[]
	setbit()

   EXPORTS:
	io_findfiledir()	Locates the file directory for a volume
	io_setfiledir()		Records address of the directory for a volume
*/

#include	<wiss.h>
#include	<io.h>

io_findfiledir(VolID, FirstPage, FileID)
register int	VolID;		/* ID of volume in question */
PID		*FirstPage;	/* first page of FileDir */
FID		*FileID;	/* internal name of FileDir */
/* Find the file ID and the first page of FileDir for a given volume.

   Returns:
	ID of first page of the directory in "FirstPage"
	ID of the file directory in "FileID"

   Side Effects:
	NONE

   Errors generated here:
	e0VOLNOTMOUNTED - volume not mounted
	e0NULLPIDPTR - null PID pointer
	e0NULLFIDPTR - null FID pointer
*/
{
	register int	i;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
		printf("io_findfiledir(VolID=%d, PID=0x%x, FID=0x%x)\n",
			VolID, FirstPage, FileID);
#endif 

	/* check input parameters */
	if (FirstPage == NULL)
		return(e0NULLPIDPTR);
	if (FileID == NULL)
		return(e0NULLFIDPTR);
        
	/* look (in the table) for the volume the page belongs to */
	for (i = 0; smPtr->VolDev[i].VDvolid != VolID; )
		if (++i >= MAXVOLS) 
                { 
                	return(e0VOLNOTMOUNTED);
                }

	SetLatch(&smPtr->VolDev[i].latch, procNum, NULL); 

	/* set up the directory info for return */
	FirstPage->Pvolid = FileID->Fvolid = VolID;
	FirstPage->Ppage = smPtr->VolDev[i].VDheader[MHEADER]->VH.VHfiledir;
	FileID->Ffilenum = FILEDIRNUM;

	ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
	return (eNOERROR);

} /* io_findfiledir */


io_setfiledir(VolID, FirstPage)
register int	VolID;	/* which volume ? */
PID	*FirstPage;	/* new first page */

/* Change the "firstpage" for FileDir on a volume - for level 2

   Returns:
	None

   Side Effects:
	Changes directory info in the volume header

   Errors generated here:
	e0VOLNOTMOUNTED - volume not mounted
	e0NULLPIDPTR - null PID pointer
	e0INVALIDPID - invalid page ID

   Bugs:
	does not fully validate a PID to see whether it is actually in use
*/
{
	register int	i;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
	{
		printf("io_setfiledir(VolID=%d, FirstPage=", VolID);
		PRINTPIDPTR(FirstPage); printf(")\n");
	}
#endif 

	/* check input parameters */
	if (FirstPage == NULL)
		return(e0NULLPIDPTR);

	/* look (in the table) for the volume the page belongs to */
	for (i = 0; smPtr->VolDev[i].VDvolid != VolID; )
		if (++i >= MAXVOLS) 
                {
                 	return(e0VOLNOTMOUNTED);
                }

	SetLatch(&smPtr->VolDev[i].latch, procNum, NULL); 

	/* update the header info and mark the header page dirty */
	if (FirstPage->Ppage < 0 || 
		FirstPage->Ppage >= smPtr->VolDev[i].VDnumpage)
        { 
		ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
		return(e0INVALIDPID);
         }
	(smPtr->VolDev[i].VDheader[MHEADER])->VH.VHfiledir = FirstPage->Ppage; 
	setbit(&smPtr->VolDev[i].VDdirty, MHEADER);	
	ReleaseLatch(&smPtr->VolDev[i].latch, procNum);
	return (eNOERROR); 

} /* io_setfiledir */
