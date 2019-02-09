
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
/* Module io_readpage: read a page from an external device

   IMPORTS:
	VolDev[]
	IO_ReadPage()

   EXPORTS:
	io_readpage()	Read a page from a device to memory
*/

#include	<wiss.h>
#include        <io.h>


io_readpage(PageID, BufPtr)
PID     *PageID;	/* identifier of the page */
PAGE    *BufPtr;	/* a memory buffer */

/* Given the identifier of a page, read it from an external device
   into a memory buffer.

   Returns:
	NONE

   Side Effects:
	Page is brought into memory

   Errors generated here:
	e0NULLPIDPTR - null PID pointer 
	e0NULLBUFPTR - null memory buffer
	e0VOLNOTMOUNTED	- volume not mounted
	e0INVALIDPID - invalid page ID

   Bugs:
	does not fully validate a PID to see whether it is actually in use
*/
{
	register int	i;
	register int	VolID;
	PID             spage;

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
	{	
		printf("io_readpage(PageID="); PRINTPIDPTR(PageID); 
		printf("Bufptr=0x%x)\n", BufPtr);
	}
#endif

	/* Assign the page to a temporary variable */
	spage= *PageID;

/* BF_event(CurrentTask->ncb_name,"io_rpage",&spage,-1); */

	/* check input parameters */
	if (PageID == NULL)
		return(e0NULLPIDPTR);
	if (BufPtr == NULL)
		return(e0NULLBUFPTR);

	/* look (in the table) for the volume the page belongs to */
	VolID = spage.Pvolid;
	for (i = 0; smPtr->VolDev[i].VDvolid != VolID; )
		if (++i >= MAXVOLS) return(e0VOLNOTMOUNTED);

	/* partially validate the PID */
	if (spage.Ppage <= 0 || spage.Ppage >= smPtr->VolDev[i].VDnumpage)
	{
#ifdef	DEBUG
	if (checkset(&Trace0, tINTERFACE))
		printf(" Invalid page number %d detected in io_readpage\n",
			spage.Ppage);
#endif
		printf("io_readpage(spage="); PRINTPIDPTR(&spage); 
		printf("Bufptr=0x%x)\n", BufPtr);

/*
		BF_dumpbuftable();
		BF_dumpbufpool();
		BF_dumpevent ();
*/

		return(e0INVALIDPID);	/* an illegal page */
	}

	/* actually read the page */
        return (IO_ReadPage(openFileDesc[i], spage.Ppage, BufPtr));

} /* io_readpage */

