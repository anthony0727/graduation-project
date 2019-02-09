
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



/* Module st_openhash - active a hash file (index)

   IMPORTS :
	st_openfile(volid, filename, mode)

   EXPORTS:
	st_openhash(volid, filename, hashno, mode)
*/

#include	<wiss.h>
#include	<st.h>

st_openhash(volid, filename, hashno, accessmode)
int	volid;		/* volume id of this file */
char 	*filename;	/* name of the data file */
int	hashno;		/* # of the hash file */
int	accessmode;	/* access mode */

/* This routine opens a hash index file
  
   RETURNS:
  	open file number of the hash file

   SIDE EFFECTS:
	None

   ERRORS:
  	None
*/
{
	char		hname[MAXFILENAMELEN];

#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
		printf("st_openhash(volid=%d, filename = %s, hashno= %d)\n", 
 				volid, filename, hashno);
#endif

	/* construct the name of the hash file */
	(void)suffixname(hname, filename, HASHSUF, hashno);

	return(st_openfile(volid, hname, accessmode));

} /* st_openhash */
