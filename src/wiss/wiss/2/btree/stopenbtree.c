
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
/* Module : st_openbtree
  	open a B-tree file for future access

   IMPORTS:
	int st_openfile (Volid, FileName, AccessMode)

   EXPORTS:
  	st_openbtree(Volid, FileName, Indexno, AccessMode)
*/

#include <wiss.h>
#include <st.h>
  
st_openbtree(volid, filename, indexno, accessmode)
int	volid;		/* volume id of this file */
char 	*filename;	/* name of the data file */
int	indexno;	/* index # of the btree index */
int	accessmode;	/* access mode */
/*
  
   RETURNS:
  	open file number of the B-tree file

   SIDE EFFECTS:
  	Updates the open file table of level 2 

   ERRORS:
  	None
*/
  
{
	char		indexfilename[MAXFILENAMELEN];

#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
		printf("st_openbtree(volid=%d, filename = %s, indexno = %d)\n", 
 				volid, filename, indexno);
#endif

	/* construct the name of the index file */
	(void)suffixname(indexfilename, filename, INDEXSUF, indexno);

	return(st_openfile(volid, indexfilename, accessmode));

} /* st_openbtree */
