
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
/* Module : st_destroyhash	
  	This routine drops a hash index file.
  
   IMPORTS:
  	st_destroyfile(Volid, filename)

   EXPORTS:
  	st_destroyhash(volid, filename, hashno)
*/

#include <wiss.h>
#include <st.h>

st_destroyhash(volid, filename, hashno, trans_id, lockup, cond)
int	volid;		/* volume id */
char 	*filename;	/* name of the file on which the hash file is built */
int	hashno;		/* # of the hash file */
int	trans_id;
short	lockup;
short	cond;
/* This routine destroys a hash index file.
  
   RETURNS:
  	eNOERROR if successful
  
   SIDE EFFECTS:
  	the descriptor for this file is removed from the file directory
  
   ERRORS:
	None
*/ 
{
	char		hashfilename[MAXFILENAMELEN];

#ifdef TRACE
	if (checkset (&Trace2, tBTREE))
		printf("st_destroyhash(volid=%d, filename=%s, hashno=%d)\n", 
 				volid, filename, hashno);
#endif

	/* construct the name of the hash file */
	(void)suffixname(hashfilename, filename, HASHSUF, hashno);

	return (st_destroyfile(volid, hashfilename, trans_id, lockup, cond));

} /* st_destroyhash */

