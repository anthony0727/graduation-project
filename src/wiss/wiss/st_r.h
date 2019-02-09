
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
#include	<r_error.h>
#include	<record.h>

/* definitions internal to level 2, record routines */

#define		EMPTYSLOT	-1	/* contents of an unused slot */
#define		REMOVEREC	-1	/* for r_slide: remove record */

/* trace settings */
#define		tSLIDE		18
#define		tRECORD		19
#define		tFRAME		20
#define		tGETPAGE	21
#define		tGETRECORD	22
#define		tSLOT		23
#define		tDATAPAGE	25
#define		tADDREC		26
#define		tPAGELINKS	27
#define		tSLICE		28
#define		tLONGITEM	29


/* maximum size of a slice, used when creating a new slice, this constant
   enforces that a slice is the only record on a data page               */
#define	MAXRECORDSIZE	((int)sizeof(DATAPAGE)-DPFIXED-(int)sizeof(TWO)-HEADERLEN)
#define SLICESIZE	MAXRECORDSIZE
#define	CRUMBSIZE	(SLICESIZE/2)

/* make a RID from a PID and a slot number */
#define MAKERID(r,p,s) 	{(r)->Rpage=(p)->Ppage;\
	(r)->Rvolid=(p)->Pvolid; (r)->Rslot=(s);}

/* A long data item consists of a directory and a number of slices in 
   which actual data are stored. The directory has a header followed by 
   a list of RID-count pairs, which are addresses and byte counts of 
   slices, respectively. Each slice physically resides on one page and 
   can be regarded as a level 2 record except that it is so big that it
   does not shared page with others.
*/
   
/* RID-count pair */
typedef struct {
	RID	rid;	/* address of this slice */
	TWO	len;	/* # of bytes in this slice */
} SLICEPTR;

/* skeleton of a directory */
typedef struct {
	FOUR		total_length;	/* total length of whole item */
	TWO 		slice_count;	/* number of slices used */
	SLICEPTR	sptr[1];	/* info about slices */
} LONGDIR;	

/* legnth of a long item directory */
#define	DIRLEN(dp)	((int)sizeof(LONGDIR)+(dp->slice_count-1)*(int)sizeof(SLICEPTR))

