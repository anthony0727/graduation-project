
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


/* types of scans */
enum	scan_type	{SEQUENTIAL, INDEXSCAN, LONGSCAN, HASHSCAN};

/* declaration for scan table entries */
typedef	struct				/* complete scan record */
{
	short int	filenum;	/* open file # of the data file */
	enum scan_type	scantype;	/* type of scan */
	short int	accessflag;	/* READ or WRITE */
	RID		rid;		/* current rid or rid of directory */
	PID             locked_page;    /* The page which was last locked */
	BOOLEXP		*boolexp;	/* search argument predicate */
	KEY		*lb;		/* lower bound of index scan */
	KEY		*ub;		/* upper bound of index scan */
	KEYINFO		*keyattr;	/* key attribute of the index */
	XCURSOR		cursor;		/* cursor of the index scan */
	short int	indexfile;	/* open file # of the index file */
	short int	deltafile;	/* open file # of the update file */
	short int	dirty_bit;	/* dirty bit for long data item */
	short int	next;		/* pointer to next scan on this file */
	int		offset;		/* cursor of a long data item */
	int		trans_id;	/* transaction id that initiated the
					   scan */
	short int	lockup;		/* TRUE if page locks are to be set */
	short int	cond;		/* TRUE if conditional locks are to be
						used */
	short int	mode;		/* lock mode of pages for scan */
} SCANINFO;
