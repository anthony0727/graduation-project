
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
/* Module io_freepage: free a page in a file

   IMPORTS:
	VolDev[]
	IO_FreePageInExt()

   EXPORTS:
	io_freepage()	Releases a page of a file
*/

#include	<wiss.h>
#include	<io.h>


io_freepage(FileID, PageID)
FID	*FileID;		/* file to free page from */
PID	*PageID;		/* page to free */

/* Mark a page as being free

   Returns:
	NONE

   Side Effects:
	Updates volume header 

   Errors generated here:
	e0NULLFIDPTR - null FID pointer 
	e0NULLPIDPTR - null PID pointer
	e0INVALIDFID - invalid file number
	e0VOLNOTMOUNTED - volume not mounted
	e0FIPIDNOTMATCH - volume IDs in the FID and PID inconsistent

   Bugs:
	does not fully validate the FID and the PID
 */
{
	register int	i, j;
        int ret_val;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
	{
		printf("io_freepage (FileID="); PRINTFIDPTR(FileID);
		printf(", PageID="); PRINTPIDPTR(PageID); 
		printf(")\n");
	}
#endif

	/* check input parameters */
	if (FileID == NULL)
		return(e0NULLFIDPTR);
	if (PageID == NULL)
		return(e0NULLPIDPTR);
	if ( (j = FileID->Fvolid) != PageID->Pvolid)
		return(e0FIDPIDNOTMATCH);	/* inconsistency */
          
	/* look up the volume */
	for (i = 0; smPtr->VolDev[i].VDvolid != j; )
		if (++i >= MAXVOLS) 
		{ 
			return(e0VOLNOTMOUNTED);
		}
	SetLatch(&smPtr->VolDev[i].latch, procNum, NULL); 

	/* check the file number */
	j = FileID->Ffilenum;
	if (j < 0 || j >= (smPtr->VolDev[i].VDheader[MHEADER])->VH.VHmaxfile)
        { 
		ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
		return(e0INVALIDFID);	/* invalid file number */
        }

        ret_val = IO_FreePageInExt(i, 1, PageID);
	ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
        return(ret_val);
}

