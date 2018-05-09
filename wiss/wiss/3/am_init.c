
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
/* Module : am_init
  	Initialize every level 0f Wiss
  
   IMPORTS:
  	AM_initscantable()		from AM_scantable.c
  	
   EXPORTS:
  	am_init();
*/

#include <wiss.h>
#include <am.h>

am_init()

/* Initialize every level 0f Wiss
  
   RETURNS:
  	None
  
   SIDE EFFECTS:
  	All the Global tables are intiialized
  
   ERRORS:
  	None
*/
  
{

#ifdef	TRACE
	if (checkset(&Trace3, tINTERFACE))
		printf("am_init()\n");
#endif

	(void)AM_initscantable();
	local_io_init();

} /* am_init */
