
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
/* Module st_volid : return the volume ID of the file

   IMPORTS :
	sys_getuser();
	ST_accessfiledesc();

   EXPORTS :
	st_openfile (volid, filename, accessmode)

*/

#include <wiss.h>
#include <st.h>

st_volid(filenum)
int	filenum;	/* open file number */

/* 
   Determines the volume id of a given file 

   Side Effects:
	NONE

   Returns:
	ID of the volume the file resides

   Errors:
	e2BADOPENFILENUM
	e2WRONGUSER
*/
{
     TWO volumeid;
  
#ifdef TRACE
	if (checkset(&Trace2,tINTERFACE))
		printf("st_volid(filenum=%d)\n",filenum);
#endif
	CHECKOFN(filenum);	 	/* check open file number */

	SetLatch(&smPtr->level2Latch, procNum, NULL);
        volumeid = F_VOLUMEID (filenum);
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(volumeid);	/* return volid */

} /* st_volid */
