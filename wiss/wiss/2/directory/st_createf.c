
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
/* Module st_createf : create a data file 

   IMPORTS:
	io_createfile(volid, numpages, extentfillfactor, newfid)
	io_destroyfile(fid)
	ST_newfiledesc(volid, filename, fid, pagefillfactor)

   EXPORTS:
	st_createfile(volid, filename, numpages, extentfillfactor, 
			pagefillfactor)
*/

#include	<wiss.h>
#include	<st.h>

st_createfile (VolID, FileName, NumPages, ExtentFillFactor, PageFillFactor)
int	VolID,			/* volume to create file on */
	NumPages,		/* how many pages to start file out with */
	ExtentFillFactor,	/* for page allocation at level 0 */
	PageFillFactor;		/* for record allocation at level 2 */
char	*FileName;		/* name of file to create */

/* Create a new file on the given device

   Returns:
	NONE

   Side Effects:
	Add entry to FileDir,  allocate extents, space in volume header.

   Errors generated here:
	NONE

*/
{
	register int	e;		/* for error checking */
	FID		fid;	/* File ID of newly-created file */

#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
	{
		printf("st_createfile (VolID=%d, FileName=%s, NumPages=%d",
			VolID, FileName, NumPages);
		printf(", ExtentFillFactor=%d, PageFillFactor=%d)\n",
			ExtentFillFactor, PageFillFactor);
	}
#endif
	e = io_createfile(VolID, NumPages, ExtentFillFactor, &fid);
	CHECKERROR(e);

	SetLatch(&smPtr->level2Latch, procNum, NULL);
	e = ST_newfiledesc(VolID, FileName, &fid, PageFillFactor);
	if (e < eNOERROR)
	{ 	
	    printf("ST_newfiledesc had error =%d\n",e);
		/* if an error occurred,  undo io_createfile */
		(void) io_destroyfile(&fid);	
                { 
			ReleaseLatch(&smPtr->level2Latch, procNum);
			return (e);
                }
	}
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(eNOERROR);

} /* st_createfile */
