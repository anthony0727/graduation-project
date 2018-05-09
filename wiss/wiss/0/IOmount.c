
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
/* Module IOmount : routines to mount and dismount volumes

   IMPORTS:
	open()
	close()
	malloc()
	free()
	shmAlloc()
	shmFree()
	IO_ReadPage()
	IO_WritePage()
	IO_checker()

   EXPORTS: 
	VolDev[]	the level 0 volume/device table
	IO_Mount	Mount a device and bring its volume header into memory
	IO_DisMount	Flush the header of a volume and dismount the device
	IO_FlushHeader	Writes out all header pages for a given volume
	IO_Format	Format a new Wiss volume
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include	<wiss.h>
#include	<io.h>
#include 	<sys/file.h>

extern	char	*malloc();
extern	int	free();
extern	char	*strcpy();
extern char * shmAlloc();

int	openFileDesc[MAXVOLS];  /* array of open file descriptors,
				one per active process */

/* Definition for open(2) */
#define	READWRITE	2
#define	WRITEONLY	1



IO_Mount(DeviceName, Vix)
char	*DeviceName;	/* External name of the device */
int	Vix;		/* volume table Vix */

/* Mount the named device and read its volume header information.
   The relevant information on the device is stored at entry "Vix"
   of the volume table.

   Returns:
	None

   Side Effects:
	Device mounted and its volume header cached in memory

   Errors:
	e0MOUNTFAILED - device mount failed
	e0NOMOREMEMORY - no more main memory
*/
{
	register i, j;		/* volatile registers */
	int	e;		/* for errors */
	int	count;		/* # of header pages */
	int	devaddr;	/* "physical" device address */
	PAGE	*buffer;	/* header block buffer */
	VolInfo	*v = &smPtr->VolDev[Vix];/* the volume table entry in concern */

#ifdef TRACE
	if (checkset(&Trace0, tVOLUMES))
		printf("IO_Mount(DeviceName=%s, Vix=%d)\n", DeviceName, Vix);
#endif

	/* mount the named "physical" device */
	if ( (devaddr = open(DeviceName, READWRITE)) < 0)
		return(e0MOUNTFAILED);

	openFileDesc[Vix] = devaddr; /* record the "physical" device address */

	/* allocate a buffer and read in the first page of the volume header */
	if ( (buffer = (PAGE *) shmAlloc(sizeof(PAGE))) == NULL)
		return(e0NOMOREMEMORY);
	e = IO_ReadPage(devaddr, HEADERADDR, buffer);
	CHECKERROR(e);
	v->VDheader[MHEADER] = buffer;

	/* calculate the number of block frames needed for each header area */
	v->VDnumext = buffer->VH.VHnumext;  /* total # of extents on volume */
	v->VDextsize = buffer->VH.VHextsize;	 /* # of pages in an extent */
	v->VDnumpage = v->VDnumext * v->VDextsize;	/* total # of pages */
	v->VDheadersize[MHEADER] = 1;
	i = (v->VDextsize - 1) / BITSPERBYTE + 1;  /* page map size in bytes */
	v->VDheadersize[PAGEMAP] = (v->VDnumext - 1) / (PAGESIZE / i) + 1;
	v->VDheadersize[EXTLINKS] = (v->VDnumext - 1) / (PAGESIZE / 2) + 1;
	v->VDheadersize[XFILEDESC] = (buffer->VH.VHmaxfile - 1) / FDPERPAGE + 1;

	/* allocate buffers for the remaining header pages */
	for (j = 0, i = MHEADER + 1; i <= XFILEDESC; j += v->VDheadersize[i++]);
	count = j + v->VDheadersize[MHEADER];
	buffer = (PAGE *) shmAlloc(sizeof(PAGE) * ( (unsigned) count) );
	if (buffer == NULL)
		return(e0NOMOREMEMORY);
	for (j = 0, i = MHEADER + 1; i <= XFILEDESC; j += v->VDheadersize[i++])
		v->VDheader[i]= buffer + j;

#ifdef	DEBUG
	if (checkset(&Trace0, tVOLUMES))
	{
		for (i = MHEADER; i <= XFILEDESC; i++)
			printf(" headersize[%d]=%d, ", i, v->VDheadersize[i]);
		printf("\n total # of header pages : %d\n", count);
	}
#endif

	/* read in the header pages and mark them as clean */
	for (i = j = v->VDheadersize[MHEADER]; i < count; i++)
	{
		e = IO_ReadPage(devaddr, HEADERADDR+i, buffer+(i-j) );
		CHECKERROR(e);
	}
	clearmap(&v->VDdirty, XFILEDESC + 1);

#ifdef	DEBUG
		e = IO_checker(Vix);
		CHECKERROR (e);
#endif

	return(eNOERROR);
	
} /* IO_Mount */


IO_DisMount(Vix)
int	Vix;		/* index to VolDev */

/* Dismount a device described by the "Vix"th entry of the volume table.
 
   Returns:
	None

   Side Effects:
	volume header on the device flushed if necessary &
	the memory buffers for the header are released 

   Errors:
	e0DISMOUNTFAILED - device dismount failed
*/
{
	int		e;	/* for returned errors */

#ifdef TRACE
	if (checkset(&Trace0, tVOLUMES))
		printf("IO_DisMount(Vix=%d)\n", Vix);
#endif

	/* flush all the modified header pages */
	e = IO_FlushHeader(Vix);
	CHECKERROR(e);

	/* "physically" dismount the device */
	if (close(openFileDesc[Vix]) != 0)
		return (e0DISMOUNTFAILED);

	/* release all the memory buffers used by the device */
	(void) shmFree((char *) smPtr->VolDev[Vix].VDheader[MHEADER]);
	(void) shmFree((char *) smPtr->VolDev[Vix].VDheader[MHEADER+1]);

	return(eNOERROR);

} /* IO_DisMount */


IO_FlushHeader(Vix)
int	Vix;			/* index into VolDev */

/* Flush header pages for the volume described by the "Vix"th in the table.

   Returns:
	NONE

   Side Effects:
	All modified header pages are written to disk

   Errors:
	None originated by this routine.
*/
{
	register i, j;		/* volatile registers */
	FOUR	pageno;		/* page # of the current header block */
	int	e;		/* for errors */
	VolInfo	*v = &smPtr->VolDev[Vix];/* address of the volume table entry */
	int	devaddr;

#ifdef TRACE
	if (checkset(&Trace0, tVOLUMES))
		printf("IO_FlushHeader (Vix=%d)\n", Vix);
#endif

#ifdef	DEBUG
	e = IO_checker(Vix);	/* Is the header ok? */
	CHECKERROR (e);
#endif

	devaddr = openFileDesc[Vix]; /* device address */
	for (i = MHEADER, pageno = HEADERADDR; i <= XFILEDESC; 
		pageno += v->VDheadersize[i++])
	{
		if (checkset(&v->VDdirty, i))	
			for (j = 0; j < v->VDheadersize[i]; j++)
			{
				e = IO_WritePage(devaddr, pageno+j,
					v->VDheader[i]+j, SYNCH, (int *) NULL);
				CHECKERROR (e);
			}
		clearbit(&v->VDdirty, i);
	}

	return (eNOERROR);

} /* IO_FlushHeader */


IO_Format(DevName, Title, volid, numext, extsize)
char	*DevName;	/* device name */
char	*Title;		/* volume title */
int	volid;		/* volume ID */
TWO	numext;		/* # of extents */
TWO	extsize;	/* # of pages in an extent */

/* Format a new volume on the named device

   Returns:
	NONE

   Side Effects:
	the storage medium on the named device is initialized

   Errors:
	e0TOOMANYVOLS - too many volume mounted
	e0NOMOREMEMORY - no more main memory
	e0MOUNTFAILED - device mount failed
*/
{
	int	i, j, e;
	int	headersize;	/* # of pages in the header */
	int	Vix;		/* volume table index */
	VolInfo *v;		/* volume table entry */
	PAGE	*vh, *buffer;	/* volume header page */
	PID	*PIDs;		/* PIDs for the volume header */
	FID	FileID;		/* file ID for the directory */
	int	devaddr;

#ifdef TRACE
	if (checkset(&Trace0, tVOLUMES))
	 printf("IO_Format(DevName=%s,Title=%s,VID=%d,numext=%d,extsize=%d)\n",
		DevName, Title, volid, numext, extsize);
#endif

	/* see if it's already mounted ? if so, dismount it */
	for (i = 0; i < MAXVOLS; i++)
		if (!strcmp(smPtr->VolDev[i].VDvolname, DevName)) break;

	if (i < MAXVOLS) 
		(void) IO_DisMount(i);
	else /* look for an empty entry */
		for (i = 0; smPtr->VolDev[i].VDvolid != NOVOL; )
			if (++i >= MAXVOLS) return(e0TOOMANYVOLS);	

	/* fill in VolDev & the main header page for those who look at them */
	v = &smPtr->VolDev[(Vix = i)];
	devaddr = open(DevName, WRITEONLY | O_CREAT, 0604);
	if (devaddr < 0) CHECKERROR(e0MOUNTFAILED);
	openFileDesc[Vix] = devaddr;
	v->VDheader[MHEADER] = vh = (PAGE *) shmAlloc(sizeof(PAGE));
	if (vh == NULL)
		return(e0NOMOREMEMORY);
	(void) strcpy(v->VDvolname, DevName);
	v->VDvolid  = vh->VH.VHvolid = volid;
	v->VDnumext = vh->VH.VHnumext = numext; /* # of extents on the volume */
	v->VDextsize = vh->VH.VHextsize = extsize; /* # of pages in an extent */
	(void) strcpy(vh->VH.VHtitle, Title);
	wsetmap(&v->VDdirty, XFILEDESC+1);	/* mark whole header dirty */

	/* fill in the rest of the main header */
	vh->VH.VHfreeext = numext;
	vh->VH.VHextlwm = 0;
	vh->VH.VHmaxfile = MAXNUMFILES;
	vh->VH.VHnumfile = 0;
	vh->VH.VHfilelwm = 0;
	vh->VH.VHfiledir = -1;

	/* calculate the number of block frames needed for each header area */
	v->VDnumpage = numext * extsize;
	v->VDheadersize[MHEADER] = 1;
	i = (extsize - 1) / BITSPERBYTE + 1;  /* page map size in bytes */
	v->VDheadersize[PAGEMAP] = (numext - 1) / (PAGESIZE / i) + 1;
	v->VDheadersize[EXTLINKS] = (2*numext - 1) / PAGESIZE + 1;
	v->VDheadersize[XFILEDESC] = (MAXNUMFILES - 1) / FDPERPAGE + 1;

	/* allocate buffers for the header pages */
	for (j = 0, i = MHEADER + 1; i <= XFILEDESC; j += v->VDheadersize[i++]);
	buffer = (PAGE *) shmAlloc(sizeof(PAGE) * ( (unsigned) j) );
	if (buffer == NULL)	
		return(e0NOMOREMEMORY);
	for (j = 0, i = MHEADER + 1; i <= XFILEDESC; j += v->VDheadersize[i++])
		v->VDheader[i] = buffer + j;
	headersize = j + 1;

	/* initialize the header - mark all files and extents free */
	buffer = v->VDheader[XFILEDESC];
	for (i = j = 0; j < vh->VH.VHmaxfile; j++)
	{
		buffer->VF[i].VFeff  = buffer->VF[i].VFextlist = -1;
		if (++i == FDPERPAGE)
			i = 0, buffer++;	/* advance to next page */
	}
	wsetmap(vh->VH.VHmap, numext);	/* mark all extents free */

	/* allocate file 0 as the directory (& the volume header too) 
	     - assume the file and the extent allocation routines
	       allocates resource from the beginning for the free list
	*/

	j = headersize + HEADERADDR;	/* number of pages to reserve */
	e = IO_AllocFile(Vix, 100, j / extsize + 1);
	CHECKERROR(e);
	/* mark the header pages unavailable */
	/* no need to use shmAlloc here */
	PIDs = (PID *) malloc( (unsigned)(sizeof(PID)*j) );
	if (buffer == NULL)	
		return(e0NOMOREMEMORY);
	FileID.Fvolid = v->VDvolid;
	FileID.Ffilenum = FILEDIRNUM;
	e = io_allocpages(&FileID, (PID *) NULL, j, PIDs);
	CHECKERROR(e);
	(void) free( (char *) PIDs);

	/* write a byte at the very end of the volume */
	e = IO_WritePage(devaddr, v->VDnumpage-1, vh, SYNCH, (int *)NULL);
	CHECKERROR(e);

	/* Flush the header */
	e = IO_DisMount(Vix);
	CHECKERROR(e);

	/* invalidate the table entry */
	smPtr->VolDev[Vix].VDvolname[0] = '\0';
	openFileDesc[Vix] = smPtr->VolDev[Vix].VDvolid = NOVOL;
	return(eNOERROR);

} /* IO_Format */


IO_Open(DeviceName, Vix)
char	*DeviceName;	/* External name of the device */
int	Vix;		/* volume table Vix */

/* Open the specified volume and store open file descriptor in
   the entry "Vix" of the openFileDesc table.

   Returns:
	None

   Side Effects:
	openfile descriptor for device stored in memory 

   Errors:
	e0MOUNTFAILED - device mount failed
*/
{
	int	devaddr;	/* "physical" device address */

#ifdef TRACE
	if (checkset(&Trace0, tVOLUMES))
		printf("IO_Open(DeviceName=%s, Vix=%d)\n", DeviceName, Vix);
#endif
	/* mount the named "physical" device */
	if ( (devaddr = open(DeviceName, READWRITE)) < 0)
		return(e0MOUNTFAILED);
	openFileDesc[Vix] = devaddr;	/* record "physical" device address */
	return(eNOERROR);
} /* IO_Open */


IO_Close(Vix)
int	Vix;		/* volume table Vix */

/* Close the file identified by openfFileDesc[Vix] 

   Returns:
	None

   Side Effects:
	closes the actual unix file
	
   Errors:
	e0DISMOUNTFAILED - device dismount failed
*/
{
	int		e;	/* for returned errors */

#ifdef TRACE
	if (checkset(&Trace0, tVOLUMES))
		printf("IO_Close(Vix=%d)\n", Vix);
#endif

	/* "physically" dismount the device */
	if (close(openFileDesc[Vix]) != 0)
		return (e0DISMOUNTFAILED);
	return(eNOERROR);
} /* IO_Close */

