
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



/* level 2 type declarations for file directory  */

/* root (hash table) page structure */
typedef struct {
	SHORTPID 	bucket[MAXBUCKETS];
} DIRROOT;

/* leaf (buckets) page structure */
#define	LEAFFIX	(sizeof(FID) + sizeof(PID)+ sizeof(SHORTPID) + 2*sizeof(TWO))
#define MAXDESCRIPTORS	(PAGESIZE - LEAFFIX) / sizeof(FILEDESC)

typedef struct {
	FILEDESC	filedesc[MAXDESCRIPTORS];
	FID		fileid;		/* level 0 file ID */
	PID		thispage;	/* PID of this page */
	SHORTPID	nextpage;	/* overflow chain */
	TWO		localdepth;	/* local depth */
	TWO		filecount;	/* # of descriptors */
} LEAFPAGE;	/* directory leaf pages format */

#define	LOWBOUND	MAXDESCRIPTORS/2	/* threshold for merging */

/* 2. -----    declarations related to volumes mounted    -----  */

typedef struct { /* volume table for holding information of directories */
	DIRROOT		*root;		/* pointer to the directory root page */
	LEAFPAGE	*leaf;		/* pointer to the leaf page buffer */
	TWO		volid;		/* the volume id */
	TWO		globaldepth;	/* global depth of the directory */
	FID		fid;		/* FID of the file directory */
	PID		pid;		/* PID of the directory root */
} DIRTBLENTRY;

#define	NOVOLID		-1		/* marker for empty entries */
