
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
/* Module : am_final
  	Finalize (clean up) every level of Wiss

   IMPORTS:
  	io_final();
  	bf_final();
  	st_final();
  
   EXPORTS:
  	int		am_final();
*/
  
#include <wiss.h>
#include <am.h>

am_final()

/* Clean up every level of Wiss
  
  	
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	None
  
   ERRORS:
  	None
*/
{
	int i;
	char *ptr;

#ifdef	TRACE
	if (checkset(&Trace3, tINTERFACE))
		printf("am_final()\n");
#endif


/*
	(void)print_stat();
*/


} /* am_final */
