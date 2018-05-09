
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



/* Module IOconcheck: volume header consistency checker 

   IMPORTS:
	countmap()
	wsetmap()
	checkset()
	setbit()
	clearbit()
	malloc()
	free()

   EXPORTS:
	IO_checker()	level 0 volume header checker
 */

#include	<wiss.h>
#include	<io.h>

extern	char	*malloc();
extern	int	free();


IO_checker(Vix)
int	Vix;	/* index to VolDev */

/* This routine checks the consistency of a volume header

   Returns:
	None

   Side effects:
	Error messages, if any, go to the standard output.

   Errors:
	e0BADHEADER - the volume header is inconsistent 
*/
{
	register i, j, l;	/* volatile registers */
	int	count;		/* counter */
	ONE	*emap;		/* the extent map */
	ONE	*tmap;		/* for validating extent allocation status */
	PAGE	*fds;		/* file descriptor page */	
	TWO	*ExtLinks = (TWO *) smPtr->VolDev[Vix].VDheader[EXTLINKS];
	VOLUMEHEADER *vh = (VOLUMEHEADER *) smPtr->VolDev[Vix].VDheader[MHEADER];

#ifdef	TRACE
	if ( checkset(&Trace0, tCONSISTENCY) )
		printf("IO_checker (Vix=%d)\n", Vix);
#endif
	
	/* check volume configuration */
	if (vh->VHnumext > sizeof(vh->VHmap) * BITSPERBYTE
		|| vh->VHnumext <= 0  || vh->VHextsize <= 0)
	{
		printf("Illegal volume configuration:");
		printf(" # of extents = %d, extent size = %d\n",
			vh->VHnumext, vh->VHextsize);
		return(e0BADHEADER);
	}

	/* check extent map */
	emap = vh->VHmap;			/* entext map address */
	count = countmap(emap, vh->VHnumext);	/* # of free extents */
	if (count != vh->VHfreeext)
	{
		printf("Extent allocation inconsistent:\n");
		printf(" map shows %d extents free, header shows %d\n",
			count, vh->VHfreeext);
		return(e0BADHEADER);
	}

	/* check file information & extent lists:
	   tmap is used to keep track of the extents that has
	   appeared in the extent list of some file examined.
	*/
	tmap = (ONE *) malloc (sizeof(vh->VHmap));
	wsetmap(tmap, vh->VHnumext);	
	fds = smPtr->VolDev[Vix].VDheader[XFILEDESC];/* file descriptor page */

	for (i = j = count = 0; j < vh->VHmaxfile; j++)
	/* j is the file number, i is the offest to the current page */ 
	{
		if (fds->VF[i].VFeff >= 0) 
		{
			count++;	/* one more file in use */
			if (fds->VF[i].VFeff > 100)
			{
				printf("Illegal fill factor %d for file %d\n",
					fds->VF[i].VFeff, j);
				return(e0BADHEADER);
			}
	
			for (l = fds->VF[i].VFextlist; l >= 0; l = ExtLinks[l])
			{ /* check extent list for file j */
				if (l >= vh->VHnumext || !checkset(tmap, l) 
					|| checkset(emap, l))
				{
					printf("File %d:bad extent list\n", j);
					return(e0BADHEADER);
				}
				clearbit(tmap, l); /* mark the extent traced */
			} /* end extent list for file j */
		}

		if (++i == FDPERPAGE)
			i = 0, fds++;	/* advance to next page */
	}

	/* # of existing files correct ? */
	if (count != vh->VHnumfile)
	{
		printf("Number of files in use inconsistent:\n");
		printf(" header shows %d, descriptor list shows %d\n",
			vh->VHnumfile, count);
		return(e0BADHEADER);
	}

	if ( vh->VHfreeext != countmap(tmap, vh->VHnumext) )
	{
		printf("Number of free extents inconsistent:\n");
		printf(" extent map shows %d, extent list shows %d\n",
			vh->VHfreeext, countmap(tmap, vh->VHnumext) );
	}

	free((char *) tmap);

	return(eNOERROR);

} /* IO_checker */

