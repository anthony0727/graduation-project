

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




/* Module IO_readwrite: routines to deal with "physical" page reads and writes

   IMPORTS:
	lseek()
	read()
	write()

   EXPORTS:
	io_diskreads	# of disk reads
	io_diskwrites	# of disk writes
	io_clearstat()	clear disk stat
	IO_ReadPage	Level 0 internal routine to read a page 
	IO_WritePage	Level 0 internal routine to write a page
 */

#include	<wiss.h>
#include	<io.h>		/* level 0 data structures & routines */
#include        <stats.h>

extern long	lseek();
extern PAGE bufferpool[];

/*
SEM	disksem;
*/

/* option codes for lseek(2) */
#define		SEEKABS		0	/* seek relative to start of file */
#define		SEEKREL		1	/* seek relative to current position */

int io_diskreads = 0;
int io_diskwrites = 0;

IO_ReadPage(devaddr, pageno, bufptr)
int	devaddr;	/* address of the "physical" device */
int	pageno;		/* "physical" address of the page */
PAGE	*bufptr;	/* where to place the page in main memory */

/* Read a page to
 * a memory buffer from a given location on a device.

   Returns:
	None

   Side Effects:
        Read a physical page into a buffer

   Errors:
	e0DEVSEEKERROR - Device seek error
	e0DEVREADERROR - Device read error
*/
{
PID	dummypage;

#ifdef TRACE
	if (checkset(&Trace0, tPHYSICALIO))
		printf("IO_ReadPage(devaddr=%d,pageno=%d,bufptr=0x%x)\n",
			devaddr, pageno, bufptr);
#endif

	io_diskreads++;


/*
	SetLatch(&smPtr->diskLatch, procNum, NULL); 
*/
	if (lseek(devaddr, (long) ( ((long) pageno) * PAGESIZE), SEEKABS) 
			< eNOERROR) 
	{
/*
	    	ReleaseLatch(&smPtr->diskLatch, procNum); 
*/
		return(e0DEVSEEKERROR);		/* seek fail */
	}

	if (read(devaddr, (char *) bufptr, PAGESIZE) != PAGESIZE)
	{
/*
	    	ReleaseLatch(&smPtr->diskLatch, procNum); 
*/
		return(e0DEVREADERROR);		/* read fail */
	}

/*
dummypage.Ppage = pageno;dummypage.Pvolid = devaddr;
BF_event(CurrentTask->ncb_name,"IO_read",&dummypage,bufptr-bufferpool); */

/*
	ReleaseLatch(&smPtr->diskLatch, procNum); 
*/
	return(eNOERROR);	/* normal completion */
}

/*ARGSUSED */
IO_WritePage (devaddr, pageno, bufptr, type, flag)
int	devaddr;	/* "physical" device address"*/
int	pageno;		/* "physical" address of the page */
PAGE	*bufptr;	/* address of the memory buffer */
int	type;		/* SYNCH or ASYNCH */
int	*flag;		/* where to return the status of the operation */

/* Write a page from a memory buffer to a given location on a device.

   Returns:
	None

   Side Effects:
        Write a page in the buffer to a page on an extern device 

   Errors:
	e0DEVSEEKERROR - Device seek error
	e0DEVWRITEERROR - Device write error
*/
{
PID	dummypage;

#ifdef TRACE
	if (checkset(&Trace0, tPHYSICALIO))
	{
		printf("IO_WritePage(devaddr=%d,pageno=%d,bufptr=0x%x,",
			devaddr, pageno, bufptr);
		printf("type=%d,flag=0x%x)\n", type, flag);
	}
#endif
	io_diskwrites++;
	no_IO_WRITES ++;
/*
	SetLatch(&smPtr->diskLatch, procNum, NULL); 
*/
	if (lseek(devaddr, (long) ( ((long) pageno) * PAGESIZE), 
			SEEKABS) < eNOERROR) 
	{
/*
		ReleaseLatch(&smPtr->diskLatch, procNum); 
*/
		return(e0DEVSEEKERROR);
	}

	if (write(devaddr, (char *) bufptr, PAGESIZE) != PAGESIZE)
	{
/*
		ReleaseLatch(&smPtr->diskLatch, procNum); 
*/
		return(e0DEVWRITEERROR);
	}
/* PHJ */

#ifdef _SYNCHDISK
	fsync(devaddr);
#endif

/*
	dummypage.Ppage = pageno;dummypage.Pvolid = devaddr;
	BF_event(CurrentTask->ncb_name,"IO_write",&dummypage,
		bufptr-bufferpool);
*/

/*
	ReleaseLatch(&smPtr->diskLatch, procNum); 
*/
	return(eNOERROR);	/* normal completion */

}

io_clearstat()
{
	io_diskreads = io_diskwrites = 0;
}
