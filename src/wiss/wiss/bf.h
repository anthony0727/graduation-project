
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


#include	<buftab.h>
#include	<sm.h>

extern  SMDESC  *smPtr;
extern	int	procNum; /* index of this process in smPtr->users table */

#define	tINTERFACE	0		/* interface rotuine trace */
#define	tBUFMANAGER	1		/* for buffer table */
#define tHASHTAB	2		/* for hash table */

extern 	TRACEFLAGS	Trace1;		/* where level 1 runtime flags are */

