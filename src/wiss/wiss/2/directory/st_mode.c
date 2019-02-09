
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
/* Module st_acessmode : query the accees mode of a file

   IMPORTS :
	None

   EXPORTS :
	st_accessmode (volid, filename, accessmode)

*/

#include <wiss.h>
#include <st.h>

st_accessmode(filenum)
int	filenum;	/* open file number */
{

/* 
   Determines the access mode of the given file 

   Side Effects:
	NONE

   Returns:
	accessmode of the active file (READ or WRITE)

   Errors:
	e2BADOPENFILENUM
	e2WRONGUSER
*/
        int filemode;

#ifdef TRACE
	if (checkset(&Trace2,tINTERFACE))
		printf("st_accessmode(filenum=%d)\n",filenum);
#endif

	CHECKOFN(filenum);	 	/* check open file number */
	SetLatch(&smPtr->level2Latch, procNum, NULL);
        filemode = smPtr->files[filenum].mode;
	ReleaseLatch(&smPtr->level2Latch, procNum);
	return(filemode);      /* return access mode */
} /* st_accessmode */

