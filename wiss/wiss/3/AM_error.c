
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
/* Module : am_error - Display errors for Wiss
  
   IMPORTS:
	io_error()
	bf_error()
	st_error()
	AM_error()

   EXPORTS:
  	am_error()
	am_errormsg()
  	am_fatalerror()
*/

/* Module : AM_error return error messages for level 3

	IMPORTS:
		None

	EXPORTS:
		AM_error()
*/

#include	<wiss.h>
#include	<am.h>

extern	char *io_error(), *bf_error(), *st_error();

char *AM_error(errorcode)
int errorcode;		/* code received */
/* return an error message for level 3

	RETURNS:
	 None

	SIDE EFFECTS:
	 None

	ERRORS:
	 None
	*/
{
#ifdef TRACE
	if (checkset(&Trace3, tINTERFACE))
		printf("AM_error(errorcode-%d)\n", errorcode);
#endif

	if (errorcode >= eNOERROR)
		return(" ");

	switch (errorcode)
	{

#include	"am_error.i"

	default:
		return("invalid Access Method Level error code");
	}

#ifdef lint
	return(" ");	/* make lint happy */
#endif

} /* AM_error */

char *am_errormsg(errorcode)
int	errorcode;		/* code received */
/* return an error message for Wiss
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
	None
  
   ERRORS:
  	None
*/
{
	int	level;		/* which level that causes the error */

#ifdef TRACE
	if(checkset(&Trace3, tINTERFACE))
		printf("am_errormsg (errorcode=%d)\n", errorcode);
#endif

	if (errorcode >= eNOERROR)
		return(" No Errors");

	level = ((-errorcode) % 100) / 10;

	switch (level)
	{
  	  case	0: 
		return(io_error(errorcode));

  	  case	1: 
		return(bf_error(errorcode));

  	  case	2: 
		return(st_error(errorcode));

  	  case	3: 
		return(AM_error(errorcode));

	  default:
		return("invalid WiSS error code");
	}

} /* am_errormsg */


am_fatalerror(routine, error)
char	*routine;			/* routine which failed */
int	error;				/* error code to interpret */
/* Print an error message for Wiss
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	Prints a message on stderr
  
   ERRORS:
  	None
*/
{

#ifdef TRACE
	if(checkset(&Trace3, tINTERFACE))
		printf("am_fatalerror (routine=\"%s\", error=%d)\n", 
			routine, error);
#endif

	(void)am_error(routine,error);
	am_final();
	exit(1);

} /* am_fatalerror */


am_error(routine, errorcode)
char	*routine;		/* calling routine */
int	errorcode;		/* code received */
/* Print an error message for Wiss
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	Prints a message on stderr
  
   ERRORS:
  	None
*/
{
	char	*s;

#ifdef TRACE
	if(checkset(&Trace3, tINTERFACE))
		printf("am_error (routine=\"%s\", errorcode=%d)\n", 
			routine, errorcode);
#endif

	s = am_errormsg(errorcode);
	printf("%s %s\n", routine, s);

} /* am_error */
