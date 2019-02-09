
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
/* Module st_error: return error message for level 2 

   IMPORTS:
	None

   EXPORTS:
	st_error()
*/

#include	<wiss.h>
#include	<st.h>

char *st_error(errorcode)
int	errorcode;		/* code received */

/* return an error message for level 2

   Returns:
	errorcode

   Side Effects:
	none

   Errors:
	NONE
*/
{

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE))
		printf("st_error (errorcode=%d)\n", errorcode);
#endif

	if (errorcode >= eNOERROR) 
		return (" ");

	switch (errorcode)
	{

#include	"../record/r_error.i"

#include	"d_error.i"

#include	"../btree/bt_error.i"

	  default:
		return("invalid Storage Level error code");
	}

#ifdef	lint
	return (" ");	/* make lint happy */
#endif

} /* st_error */
