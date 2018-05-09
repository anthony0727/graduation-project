
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
/* Module st_firstfile : routine to compute the ID of the first record
	
   IMPORTS:
	st_nextfile(filenum, ridptr, nextridptr, trans_id, lockup, mode, cond);

   EXPORTS:
	st_firstfile(filenum, firstrid, trans_id, lockup, mode, cond)

*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

st_firstfile(filenum, firstrid, trans_id, lockup, mode, cond)
int	filenum;		/* open file number */
RID	*firstrid;		/* *RID */
int     trans_id;
int     lockup;
LOCKTYPE mode;
int     cond;

/* Compute the ID of the first record of the file.

   Returns:
	ID of the first record (via firstrid)

   Side Effects:
	None

   Errors:
	None

*/

{
	int e;

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE))
		printf("st_firstfile(filenum=%d,*rid=0x%x)\n",filenum,firstrid);
#endif
/*
	 printf("st_firstfile, transid=%d, lockup=%d, mode=%d, cond=%d\n",
		 trans_id, lockup, mode, cond);
*/

	/* let st_nextfile does the dirty work */
        e = st_nextfile(filenum, (RID *) NULL, firstrid, trans_id, lockup, 
		mode, cond);
        return(e);
} /* st_firstfile */
