
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
/* Module io_destroyf: remove a file 

   IMPORTS:
	VolDev[]
	IO_FreeFile()

   EXPORTS:	
	io_destroyfile()	remove an existing file
*/

#include	<wiss.h>
#include        <io.h>

io_destroyfile(FileID)
FID	*FileID;	/* which file to remove */
/* Remove a file and release all its resources

   Returns:
	None

   Side Effects:
	File entry is obliterated, extents are freed.

   Errors generated here:
	e0VOLNOTMOUNTED - volume not mounted
	e0NULLFIDPTR - null FID pointer
	e0INVALIDFID - invalid FID
*/
{
	register i, j;
        int ret_val;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
	{	printf("io_destroyfile (FileID=");
		PRINTFIDPTR(FileID); printf(")\n");
	}
#endif

	/* check input parameter */
	if (FileID == NULL)
		return(e0NULLFIDPTR);


	/* look up the volume */
	for (i = 0, j = FileID->Fvolid; smPtr->VolDev[i].VDvolid != j; )
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
	    	return(e0INVALIDFID);
        }
	ret_val = IO_FreeFile(i, j);
	ReleaseLatch (&smPtr->VolDev[i].latch, procNum);
	return (ret_val);

} /* io_destroyfile */
