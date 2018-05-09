
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


/* definitions internal to level 2. - file directory and file table */

/* level 2 trace flags */
#define	tFILEDIR	1
#define	tSTAT		30
#define	tEXTHASH	31

#define	MAXFILENAMELEN	40	

#define	DATAFILE	0
#define	INDEXFILE	1
#define	HASHFILE	2
#define	UNIXFILE	3

/* structure of file descriptors */
typedef	struct {
	char	filename[MAXFILENAMELEN];/* file name */
	FID	fileid;		    /* file id  of file */
	FOUR	serverofn;	    /* open file number of file on server */
	FOUR	filesize;	    /* # of bytes in the file */
	FOUR	numpages;	    /* current # of pages in the file */
	FOUR	card;		    /* current # of records or keys */
	FOUR	flags;		    /* (permission) flags */
	FOUR	owner;		    /* owner of the file */
	FOUR	filetype;	    /* type of the file */
	PID	firstpage;	    /* firstpage of file */
	PID	lastpage;	    /* lastpage of file */
	PID	rootpage;	    /* root of a B-tree file -
					this never changes !!! */
	FOUR	pff;		    /* page fill factor in % */
} FILEDESC;

enum    bt_oper    { BT_READ, BT_INSERT, BT_DELETE, BT_INSERT_DELETE, BT_NONE };

/* open file table */
#define	CLEAN		0
#define	DIRTY		1
#define	NOTINUSE	2

/* bit assignment for the status (flag) word */
#define	OWNERREAD	0400
#define	OWNERWRITE	0200
#define	OTHERREAD	04
#define	OTHERWRITE	02
#define	DEFAULT_PERM	(OWNERREAD | OWNERWRITE | OTHERREAD)
#define	ALL_PERM	(OWNERREAD | OWNERWRITE | OTHERREAD | OTHERWRITE)


