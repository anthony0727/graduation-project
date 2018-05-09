
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
/* Module IOfiman: file management 

   IMPORTS:
	VolDev[]
	setbit()
	malloc()
	free()
	IO_AllocExtents()
	IO_FreeExtents()

   EXPORTS:
	IO_AllocFile()	Creates a new file and allocates some extents for it
	IO_FreeFile()	Removes a file and releases the extents it owns

   NOTES:
	IO_AllocFile() and IO_FreeFile() deals with the allocation
 	and deallocation of file descriptors, respectively.
	They are responsible for maintaining the file descriptor list
	and the extent links in the volume header.
*/

#include	<wiss.h>
#include	<io.h>


extern	char	*malloc();
extern	int	free();

int IO_AllocFile(Vix, EFF, NumExt)
int	Vix;		/* volume table index */
int	EFF;		/* extent fill factor */
int	NumExt; 	/* # of extents needed */

/* Allocate a new file from the volume header.

   Returns:
	a file number

   Side Effects:
	a new file is created

   Errors generated here:
	e0TOOMANYFILES	- too many files already exist on the volume
*/
{
	register int	i;
	int		e;		/* for error checking */
	int		filenum;	/* a file number */
	TWO		*ExtLinks;	/* extent links array */
	TWO		*Exts;		/* array of extent numbers */
	VOLFILE		*fd;		/* file descriptor pointer */
	PAGE		*fds;		/* file descriptor page */
	VOLUMEHEADER	*vh = (VOLUMEHEADER *) smPtr->VolDev[Vix].VDheader[MHEADER];
				/* pointer to the main volume header */
#ifdef TRACE
	if (checkset(&Trace0,tFILE) )
		printf("IO_AllocFile(Vix=%d,EFF=%d,NumExt=%d)\n", 
			Vix, EFF, NumExt);
#endif

	/* look for an empty file descriptor entry */
	filenum = vh->VHfilelwm;	/* where to start the search */
	fds = (smPtr->VolDev[Vix].VDheader[XFILEDESC]+filenum/FDPERPAGE);
	for (i = filenum % FDPERPAGE; fds->VF[i].VFeff > 0; )
	{ 
		if (++filenum >= vh->VHmaxfile)
			return(e0TOOMANYFILES);	/* no descriptors available */
		if (++i == FDPERPAGE)
			i = 0, fds++;	/* advance to next page */
	}
	fd = &(fds->VF[i]);	/* address of the descriptor */

	/* allocate extents for the file and link them up */
	Exts = (TWO *) malloc( (unsigned) (NumExt * 2) );
	e = IO_AllocExtents(Vix, NumExt, Exts);	/* allocate the extents */
	CHECKERROR(e);
	ExtLinks = (TWO *) (smPtr->VolDev[Vix].VDheader[EXTLINKS]);
	fd->VFeff = EFF;
	fd->VFextlist = Exts[0];	/* start of the extent list */
	for (i = 1; i < NumExt; i++)
		ExtLinks[Exts[i - 1]] = Exts[i];
	ExtLinks[Exts[i - 1]] = -1;	/* end of list */
	(void) free ( (char *) Exts);

	setbit(&smPtr->VolDev[Vix].VDdirty, EXTLINKS);
	setbit(&smPtr->VolDev[Vix].VDdirty, XFILEDESC);

	/* update the control information in the main header */
	vh->VHnumfile++;
	vh->VHfilelwm = filenum + 1;
	setbit(&smPtr->VolDev[Vix].VDdirty, MHEADER);

	return(filenum);

} /* IO_AllocFile */


IO_FreeFile(Vix, filenum)
int	Vix;		/* volume table index */
int	filenum;	/* file to be removed */

/* Remove a file from volume header, and free all its extents.

   Returns:
	None

   Side Effects:
	file descriptor list and extent links updated

   Errors generated here:
	e0FILENOTINUSE - invalid file number
*/

{
	register i;
	register NumExt;	/* # of extents to release */
	register TWO *ExtLinks;	/* extent links */
	TWO	*Exts;		/* array of extents to release */
	VOLFILE	*fd;		/* address of the file descriptor */

#ifdef TRACE
	if ( checkset(&Trace0,tFILE) )
		printf("IO_FreeFile(Vix=%d,filenum=%d)\n", Vix, filenum);
#endif

	/* calculate the address of the file descriptor & validate the file */
	fd = &( (smPtr->VolDev[Vix].VDheader[XFILEDESC]+filenum/FDPERPAGE)->
		VF[filenum%FDPERPAGE] );
	if (fd->VFeff < 0)
		return(e0FILENOTINUSE);	/* not a vaild file */

	/* release all the extents in the file */
	ExtLinks = (TWO *) smPtr->VolDev[Vix].VDheader[EXTLINKS];
	for (i = fd->VFextlist, NumExt = 0; i >= 0; i = ExtLinks[i], NumExt++);
	Exts = (TWO *) malloc( (unsigned) (NumExt * 2) );
	for (i = fd->VFextlist, NumExt = 0; i >= 0; i = ExtLinks[i], NumExt++)
		Exts[NumExt] = i;
	(void) IO_FreeExtents(Vix, NumExt, Exts);
	(void) free( (char *) Exts);

	/* mark the file not in use */
	fd->VFeff = -1;
	fd->VFextlist = -1;
	setbit(&smPtr->VolDev[Vix].VDdirty, XFILEDESC);

	/* update the contorl info in the volume header */
	smPtr->VolDev[Vix].VDheader[MHEADER]->VH.VHnumfile--;
	if (filenum < smPtr->VolDev[Vix].VDheader[MHEADER]->VH.VHfilelwm)
		smPtr->VolDev[Vix].VDheader[MHEADER]->VH.VHfilelwm = filenum;
	setbit(&smPtr->VolDev[Vix].VDdirty, MHEADER);

	return(eNOERROR);

} /* IO_FreeFile */

