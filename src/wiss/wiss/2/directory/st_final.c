
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
/* Module st_final: level 2 finalization routine 

   IMPORTS:
	ST_filedirfinal()
	filetable[]
	files[]

   EXPORTS:
	st_final()
*/

#include <wiss.h>
#include <page.h>
#include <sm.h>

SMDESC  *smPtr;

#define	ROOT(i)		(smPtr->dirtable[i].root)
#define	LEAF(i)		(smPtr->dirtable[i].leaf)
#define VOLID(i)	(smPtr->dirtable[i].volid)
#define	ROOTFID(i)	(smPtr->dirtable[i].fid)
#define	DROOTPID(i)	(smPtr->dirtable[i].pid)
#define	GDEPTH(i)	(smPtr->dirtable[i].globaldepth)
#define	BUCKET(i,j)	(smPtr->dirtable[i].root->bucket[j])


st_final()
/* level 2 finalization : flush the file table

   Returns:
	None

   Side Effects:
	clear the file table

   Errors:
	None
*/
{
	register i;

#ifdef TRACE
	if (checkset(&Trace2, tINTERFACE))
		printf("st_final()\n");
#endif

	ST_filedirfinal();

	for (i = 0; i < MAXOPENFILES; i++) 
		smPtr->filetable[i].status = NOTINUSE;

	for (i = 0; i < MAXOPENFILES; i++) 
		smPtr->files[i].ptr = NIL;

	return(eNOERROR);

} /* st_final */


ST_filedirfinal()
/* This cleans up the directory table

   Returns:
	None

   Side Effects:
	table cleaned

   Errors:
	None
*/
{
	register int	i;	/* table index */

#ifdef TRACE
	if (checkset(&Trace2,tFILEDIR))
		printf("ST_filedirfinal()\n");
#endif

	for (i = 0; i < MAXVOLS; i++) 
		if (VOLID(i) != NOVOLID) 
		{
			shmFree((char *) ROOT(i));
			shmFree((char *) LEAF(i));
			ROOT(i) = NULL;
			LEAF(i) = NULL;
			VOLID(i) = NOVOLID;		/* mark it empty */
		}

	return(eNOERROR);

} /* ST_filedirfinal */
