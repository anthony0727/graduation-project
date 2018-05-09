
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


/* page.h: formats of physical pages */

#define	TITLEMAX	60		/* max length of a device's title */

/* fixed overhead of a data page */
#define	DPFIXED	(5*(int)sizeof(FOUR) + (int)sizeof(FID) + (int)sizeof(PID) )

typedef struct { /* data page */ 
	char	data [PAGESIZE - DPFIXED];
	FOUR	slot[1];		/* slot 0 -- index backwards */
	PID	thispage;		/* address of current page */
	FOUR	free;			/* pointer to data free area */
	FID	fileid;			/* who this page belongs to */
	FOUR	ridcnt;			/* number of RID slots onpage */
	SHORTPID prevpage;		/* address of previous page */
	SHORTPID nextpage;		/* address of next page */
} DATAPAGE;
  
typedef struct { /* btree control information */
	TWO	enddata;	/* offset to end of the data section */
	TWO	startdata;	/* offset to the start of data
				** This will be 0 for all but the root */
	TWO	numoffsets;	/* number of offsets */
	FID	fileid;		/* owner of this page */
	PID	thispage;	/* points to this page  */
	TWO	numfree;	/* number of free bytes */
	ONE	keytype;	/* type as in int, char... */
	ONE	pagetype;	/* root,node,leaf,overflow */
	ONE	treetype;	/* index, link, version */
	ONE	unique;		/* type of index */
	SHORTPID next;		/* next page-same level */
	SHORTPID prev;		/* previous page-same level */
	SHORTPID overflow;	/* overflow ptr for leaves */
	SHORTPID pid0;		/* first ptr, so no key */
} BTCONTROL;

/* offset to the end of data. TWO is for slot[1]. */
#define ENDDATA		PAGESIZE - (int)sizeof(BTCONTROL) 

typedef struct {	/* btree page */
	char		data[ENDDATA-sizeof(TWO)];
	TWO		slot[1];	/* first slot on the page */
	BTCONTROL	btcontrol;      /* control information    */
} BTREEPAGE;


typedef struct { /* volume header page */
	char	VHtitle[TITLEMAX];/* title of volume--human ID */
	TWO	VHvolid;	/* unique ID of this volume */
	TWO	VHextsize;	/* number of pages in an extent */
	TWO	VHnumext;	/* total number of extents in vol */
	TWO	VHfreeext;	/* number of free extents in vol */
	TWO	VHextlwm;	/* low water mark for free extents */
	TWO	VHmaxfile;	/* max Number of files */
	TWO	VHnumfile;	/* number of files currently */
	TWO	VHfilelwm;	/* low water mark for free file descriptors */
	SHORTPID VHfiledir;	/* 1st page of FileDir */
	ONE	VHmap		/* start of extent bit map */
		[PAGESIZE-8*(int)sizeof(TWO)-TITLEMAX-(int)sizeof(SHORTPID)];
/* The low water marks are used for speeding up allocation of free extents 
   or file descriptors. Entries below a low water mark are guaranteed to 
   be allocated, and thus wasteful searching is avoided.
*/
} VOLUMEHEADER;

typedef struct	{
	TWO	VFeff;		/* extent fill factor */
	TWO	VFextlist;	/* list of extents allocated */
} VOLFILE;

#define	FDPERPAGE	(PAGESIZE/(int)sizeof(VOLFILE))

/**** And for the grand finale ****/
typedef union {
	VOLUMEHEADER	VH;			/* volume header format */
	VOLFILE		VF [FDPERPAGE];		/* file descriptors pages */
	DATAPAGE	DP;			/* data format */
	BTREEPAGE	BT;			/* btree page format */
	ONE		CH [PAGESIZE];		/* byte format */
	TWO		SH [PAGESIZE / (int)sizeof(short)];	/* short integers */
} PAGE;

