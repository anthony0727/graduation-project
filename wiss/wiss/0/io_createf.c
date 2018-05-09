
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
/* Module io_createf: create a new file 

   IMPORTS:
	VolDev[]
	IO_AllocFile()

   EXPORTS:
	io_createfile()		Creates a new file
*/

#include	<wiss.h>
#include	<io.h>


io_createfile(VolID, NumPages, EFF, NewFile)
int	VolID;		/* Volume to create file on */
int	NumPages;	/* Number of pages initially */
int	EFF;		/* % of an extent to keep filled */
FID	*NewFile;	/* returned FID of the new file */

/* Create a new file on a volume with at least "NumPages" pages.

   Returns:
	ID of the newly-created file in NewFile

   Side Effects:
	Adds file to the volume, allocates extents.

   Errors:
	e0VOLNOTMOUNTED	- volume not mounted
	e0NULLFIDPTR - null FID pointer
	e0TOOMANYFILES - too many files on the volume
	e0NOSPACEONDISK - no more free disk space
*/
{
	register int	i;		/* volume table index */
	int		j;
	int		NumExt;		/* number of extents we need */
	int		FileNum;	/* internal file number */
	VOLUMEHEADER	*vh;		/* volume header */

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
 	     printf("io_createfile(VolID=%d,NumPages=%d,EFF=%d,NewFile=0x%x)\n",
			VolID, NumPages, EFF, NewFile);
#endif

	/* check the input parameter */
	if (NewFile == NULL)
		return(e0NULLFIDPTR);
        
	/* look up the volume */
	for (i = 0; smPtr->VolDev[i].VDvolid != VolID; )
		if (++i >= MAXVOLS) 
                 {
                   	return(e0VOLNOTMOUNTED);
                 }
	SetLatch(&smPtr->VolDev[i].latch, procNum, NULL); 
	vh = (VOLUMEHEADER *) smPtr->VolDev[i].VDheader[MHEADER]; 

	/* ignore bad hints */
	if (EFF <= 0 || EFF > 100)
		EFF = 100;
	if (NumPages <= 0) 
		NumPages = 1;		/* at least one page */

	/* calculate how many extents we need */
	j = EFF*vh->VHextsize/100;
	if (j < 1) j = 1;
	NumExt = (NumPages - 1) / j + 1;

	/* check if there are enough resources - files, extents */
	if (vh->VHnumfile >= vh->VHmaxfile)
        {
		ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
	 	return(e0TOOMANYFILES);
        }
	if (vh->VHfreeext < NumExt)
        { 
		ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
		return(e0NOSPACEONDISK);
        }

	/* allocate a new file and return its identifier */
	FileNum = IO_AllocFile(i, EFF, NumExt);
/*
	CHECKERROR(FileNum);
*/
	CHECKERROR_RLATCH(FileNum,&smPtr->VolDev[i].latch,procNum);
	NewFile->Fvolid = VolID;	/* synthesize a FID */
	NewFile->Ffilenum = FileNum;
	ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
	return (eNOERROR);

} /* io_createfile */

