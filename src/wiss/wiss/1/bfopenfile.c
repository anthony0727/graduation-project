
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
/* Module bf_openfile:
	This routine notifies the buffer manager that a new file is being
	open. Currently, this routine does nothing.

   Imports :
	None

   Exports :	
	bf_openfile(filenum, mode)

   Returns :
	None

   Errors:
	None

*/

#include <bf.h>

/*ARGSUSED*/
bf_openfile(filenum, mode)
int	filenum;
int	 mode;		/* mode may be R for readonly or W for read/write*/
{

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_openfile(filenum=%d, mode=%d)\n", filenum, mode);
#endif

	return(eNOERROR);

} /* end of bf_openfile */

