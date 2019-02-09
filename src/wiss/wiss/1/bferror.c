
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
/* Module bf_error: 
	interpret an error message

   IMPORTs : 
	NONE

   EXPORTS:
	char *bf_error()

   Returns:
	None

   Side Effects:
	None

   Errors:
	None
*/

#include <bf.h>

char *bf_error(errorcode)
int	errorcode;		/* code received */

{
#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_error(errorcode=%d)\n", errorcode);
#endif

	if (errorcode >= eNOERROR) 
		return(" ");	/* no errors */

	switch (errorcode) {
#include	"bferror.i"
	  default:
		return("Invalid Buffer Manager error code\n");
	}

#ifdef	lint
	return(" ");
#endif

} /* bf_error */

