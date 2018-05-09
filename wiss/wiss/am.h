
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



/* definitions internal to level 3. */
#include	<page.h>
#include	<am_error.h>
#include	<scaninfo.h>

#include	<sm.h>
extern  SMDESC  *smPtr;

/* level 3 trace flag */
#define		tINTERFACE	0
#define		tBOOLEAN	1
#define		tSCANTABLE	2
#define		tCOMMIT		3

/* types of update records in the update file */
enum	upd_type	{INSERT, DELETE, UPDATE};

/* skeleton of a update record for index differential file */
typedef struct
{
	enum upd_type	type;		/* INSERT, UPDATE or DELETE */
	RID		datarid;	/* RID of the record modified */
	char		image[1];	/* start of the new record image */
} UPDRECORD;

#define	UPDHEADERLEN	((int)sizeof(enum upd_type)+(int)sizeof(RID))

extern TRACEFLAGS	Trace3;
