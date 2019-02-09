
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
/* Module io_writepage: write a page to an external device

   IMPORTS:
	VolDev[]
	IO_WritePage()

   EXPORTS:
	io_writepage()	Writes a page to memory
*/

#include	<wiss.h>
#include        <io.h>


io_writepage(PageID, BufPtr, wtype, wflag)
PID     *PageID;	/* identifier of the page */
PAGE    *BufPtr;	/* where the new image is stored */
int     wtype;		/* SYNCH or ASYNCH */
int	*wflag;		/* status code for ASYNCH write */

/* Write a page from a memory buffer to an external device.

   Returns:
	NONE

   Side Effects:
	a page is written to device

   Errors generated here:
	e0NULLPIDPTR - null PID pointer 
	e0NULLBUFPTR - null memory buffer
	e0VOLNOTMOUNTED - volume not mounted
	e0INVALIDPID - invalid page ID

   Bugs:
	does not fully validate a PID to see whether it is actually in use
*/
{
	register int	i, j;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
	{	
		printf("io_writepage(PID="); PRINTPIDPTR(PageID); 
		printf("Bufptr=0x%x,type=%d,flag=0x%x)\n",BufPtr, wtype, wflag);
	}
#endif

	/* check input parameters */
	if (PageID == NULL)
		return(e0NULLPIDPTR);
	if (BufPtr == NULL)
		return(e0NULLBUFPTR);

	/* look (in the table) for the volume the page belongs to */
	j = PageID->Pvolid;
	for (i = 0; smPtr->VolDev[i].VDvolid != j; )
		if (++i >= MAXVOLS) return(e0VOLNOTMOUNTED);

	/* partially validate the PID */
	j = PageID->Ppage;
	if (j <= 0 || j >= smPtr->VolDev[i].VDnumpage)
	{
#ifdef	DEBUG
	if (checkset(&Trace0, tINTERFACE))
		printf(" Invalid page number %d detected in io_writepage\n",
			PageID->Ppage);
#endif
		printf("io_writepage(PID="); PRINTPIDPTR(PageID); 
		printf("Bufptr=0x%x,type=%d,flag=0x%x)\n",BufPtr, wtype, wflag);
		return(e0INVALIDPID);	/* an illegal page */
	}

	/* actually writing out the page */
	return (IO_WritePage(openFileDesc[i], j, BufPtr, wtype, wflag));

} /* io_writepage */

