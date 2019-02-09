
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
/* Module : st_dropbtree	
  	This routine destroys a B-tree file.
  
   IMPORTS:
  	st_destroyfile(Volid, filename)

   EXPORTS:
  	st_dropbtree(volid, filename, indexno)
*/

#include <wiss.h>
#include <st.h>
#include        <lockquiz.h>


st_dropbtree(volid, filename, indexno, trans_id, lockup, cond)
int	volid;		/* volume id */
char 	*filename;	/* name of the file on which the index is built */
int	indexno;	/* index # of the B-tree file */
int	trans_id;
short	lockup;
short	cond;
/* This routine destroys a B-tree file.
  
   RETURNS:
  	eNOERROR if successful
  
   SIDE EFFECTS:
  	the descriptor for this file is removed from the file directory
  
   ERRORS:
	None
*/ 
{
	char		indexfilename[MAXFILENAMELEN];

#ifdef TRACE
	if (checkset (&Trace2, tINTERFACE))
		printf("st_dropbtree(volid=%d, filename=%s, indexno=%d)\n", 
 				volid, filename, indexno);
#endif

	/* construct the name of the index file */
	(void)suffixname(indexfilename, filename, INDEXSUF, indexno);

	return (st_destroyfile(volid, indexfilename, trans_id, lockup, cond));

} /* st_dropbtree */

