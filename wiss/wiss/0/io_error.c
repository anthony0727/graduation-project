
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
/* Module io_error: return error messages for level 0 

   IMPORTS:
	None

   EXPORTS:
	io_error()	return address of an error message
*/

#include	<wiss.h>
#include	<io.h>


char *io_error(errorcode)
int	errorcode;		/* code received */

/* Return an error message for level 0

   Returns:
	NONE

   Side Effects:
	Prints a message on stderr

   Errors:
	NONE
*/
{

#ifdef TRACE
	if (checkset(&Trace0, tINTERFACE))
		printf("io_error (errorcode=%d)\n", errorcode);
#endif

	if (errorcode >= eNOERROR) 
		return(" ");		/* not an error */

	switch (errorcode)
	{

#include	"IOerror.i"

	  default:
		return("Invalid I/O error code");
	}

#ifdef	lint
	return(" ");		/* make lint happy */
#endif

} /* io_error */

