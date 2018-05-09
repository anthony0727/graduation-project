
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
/* module st_init: level 2 initialization routine 

   IMPORTS:
	ST_filedirinit()

   EXPORTS:
	st_init()
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

st_init()

/* level 2 initialization : initialize the tables of level 2

   Returns:
	None

   Side Effects:
	clear the file table

   Errors:
	None
*/
{
	register int i;		/* table index */

	ST_filedirinit();

	for (i = 0; i < MAXOPENFILES; i++) 
		smPtr->filetable[i].status = NOTINUSE;

	for (i = 0; i < MAXOPENFILES; i++) 
		smPtr->files[i].ptr = NIL;
	
	InitLatch(&smPtr->level2Latch);

	return (eNOERROR);
} /* st_init */



ST_filedirinit()
/* This initializes the table for maintaining file directories

   Returns:
	None

   Side Effects:
	directory table initialized

   Errors:
	None
*/
{
	register int	i;	/* table index */

	for (i = 0; i < MAXVOLS; i++) VOLID(i) = NOVOLID; /* clear table */

	return(eNOERROR);

} /* ST_filedirinit */

