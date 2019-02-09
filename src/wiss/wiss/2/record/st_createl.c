
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
/* Module st_createlong : routine to create a long data item.

   IMPORTS :
	st_appendrecord(filenum, recaddr, reclen, newridptr, trans_id, lockup, cond)

   EXPORTS :
	st_createlong(filenum, rid, trans_id, lockup, cond) 

*/

#include 	<wiss.h>
#include	<st.h>

static LONGDIR  zerodbuf;

st_createlong(filenum, ridptr, trans_id, lockup, cond)
int	filenum;	/* file number */
RID	*ridptr;	/* for returning the RID of directory */
int     trans_id;
short   lockup;
short   cond;

/* Create a long data item, and return the RID of its directory.

   Returns:
	RID of the directory (via ridptr)

   Side Effects:
	None

   Errors:
	e2NULLRIDPTR : the pointer for returning RID is null.
*/
{
	int		e;		/* for returned errors */
	LONGDIR		dirbuf;		/* buffer for directory */

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE))
		printf("st_createlong(filenum=%d,RID=0x%x)\n",filenum,ridptr);
#endif

	/* check file permission */
	CHECKOFN(filenum);
	CHECKWP(filenum);

	if (ridptr == NULL) return(e2NULLRIDPTR);

	/* initialize byte count and slice count to zeros */
	dirbuf = zerodbuf;

	/* initialize the directory as an oridinary record */
	e = st_appendrecord(filenum, (char *)&dirbuf, sizeof(dirbuf), ridptr, 
		trans_id, lockup, cond);
	return(e);

}	/* st_createlong */

