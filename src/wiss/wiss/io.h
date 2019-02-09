
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
/* IO.h: shared data structures and definitions for level 0 */
#include	<page.h>
#include	<io_error.h>
#include	<sm.h>

extern  SMDESC  *smPtr;
extern	int	procNum; /* index of this process in smPtr->users table */
extern	int	openFileDesc[]; /* open file descriptors for each mounted vol */
				/* one of these arrays per process */

#define	MAXNUMFILES	4096	/* max # of files in a volume */

/* predefined system addresses */
#define	HEADERADDR	0	/* where the volume Header starts (page #) */
#define	FILEDIRNUM	0	/* FileDir is always file number 0 */

/* constants for tracing */
#define	tINTERFACE	0	/* interface routines (io_...) */
#define	tCONSISTENCY	1	/* consistency checker, volume initialization */
#define	tFILE		2	/* file creation and destruction */
#define	tVOLUMES	3	/* volumes on/off line, id's, etc */
#define	tALLOCATION	4	/* extent/page (de)allocation */
#define	tPHYSICALIO	5	/* physical I/O routines & details */

/* for naming the areas in a volume header */
#define	MHEADER		0
#define	EXTMAP		0
#define	PAGEMAP		1
#define	EXTLINKS	2
#define	XFILEDESC	3

extern TRACEFLAGS	Trace0;

