
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



/* definitions internal to level 2. */

#include	<st_r.h>
#include	<st_bt.h>
#include	<page.h>
#include	<sm.h>

extern SMDESC *smPtr;
extern int	procNum;

/* level 2 trace flag */
#define		tINTERFACE	0

#define	ALIGN		((int)sizeof (ALIGNTYPE))	/* for aligning data */
#define	MAKEALIGN(l)	if (l%ALIGN) l+=(l<0) ? abs(l%ALIGN) : ALIGN-(l%ALIGN)
	/* MAKEALIGN: for positive numbers, determines how much room
		(aligned) a byte count needs.
			for negative numbers, determines how far over
		we move something to eat up that many bytes
	*/

extern TRACEFLAGS	Trace2;

/* This macro is for checking the validity of an open file number */
#define	CHECKOFN(ofn) 	if ((ofn)>=MAXOPENFILES||(ofn)<0||\
			smPtr->files[ofn].ptr==NIL) return(e2BADOPENFILENUM);

/* This macro is for checking if a file is open with write permission */
#define	CHECKWP(ofn) 	if (F_PERM(ofn) != WRITE) return(e2NOPERMISSION)

