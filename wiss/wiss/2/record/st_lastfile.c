
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
/* Module st_lastfile: routine to compute the ID of the last record

   IMPORTS :
	st_prevfile(filenum, ridptr, prevridptr, trans_id, lockup, mode, cond)

   EXPORTS :
	st_lastfile(filenum, lastrid, trans_id, lockup, mode, cond)

*/

#include 	<wiss.h>
#include	<st.h>
#include        <lockquiz.h>

st_lastfile(filenum, lastrid, trans_id, lockup, mode, cond)
int		filenum;	/* open file number */
RID		*lastrid;	/* *RID */
int             trans_id;       
short           lockup;        
LOCKTYPE        mode;
short		cond;

/* Compute the ID of the last record.

   Returns:
	the ID of the last record (via lastrid)

   Side Effects:
	None

   Errors:
	None

*/

{

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE)) {
		printf("st_lastfile(filenum=%d", filenum);
		PRINTRIDPTR(lastrid); printf(")\n");
	}
#endif

	/* let st_prevfile do the dirty work */
	/* especially all the mess with locking of the pages */
        return(st_prevfile(filenum, (RID *) NULL, lastrid, trans_id, lockup, 
		mode, cond));

}	/* st_lastfile */

