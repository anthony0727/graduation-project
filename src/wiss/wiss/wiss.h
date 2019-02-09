
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



/* added for operation on a SUN system */
#define	movebytes(to,from,length)	bcopy(from,to,length)

/* system parameters */
#define PAGESIZE 	1048576
#define MAXBUFS		512	/* default # of page buffers */
#define	MAXVOLS		16	/* max # of mounted volumes */
#define	MAXOPENFILES	100	/* max # of open files	*/
#define	MAXFIELD	256	/* max field length */

/* macro definitions and data structures used by all WiSS routines */

#define	eNOERROR	0		/* non-error "error" code */
#ifndef NULL
#define	NULL		0		/* null memory address */
#endif
#define	NIL		-1		/* nil index */

/* for checking for errors and returning */
#define		CHECKERROR(a)	if ((int)(a) < eNOERROR) return ((int)(a))
/* for sending a semaphore in addition to returning a value */
#define  CHECKERROR_RLATCH(a,b,c)   if ((int)(a) < eNOERROR) \
		  { ReleaseLatch(b,c);  return ((int)(a));}

/* FIDEQ,PIDEQ,RIDEQ compare FIDs PIDs and RIDs.  Pass them an actual
   structure, not a pointer. */
#define		FIDEQ(a,b)	((a).Fvolid == (b).Fvolid && \
				 (a).Ffilenum == (b).Ffilenum)
#define		FIDCLEAR(a)   	(a).Fvolid= -1;(a).Ffilenum= -1
#define		TESTFIDCLEAR(a)	((a).Fvolid == -1 && (a).Ffilenum == -1)
#define		PIDEQ(a,b)	((a).Ppage  == (b).Ppage && \
				 (a).Pvolid == (b).Pvolid)
#define		PIDCLEAR(a)	(a).Ppage= -1;(a).Pvolid= -1
#define		TESTPIDCLEAR(a)	((a).Ppage == -1 && (a).Pvolid == -1)
#define		RIDEQ(a,b)	((a).Rpage  == (b).Rpage  && \
				 (a).Rvolid == (b).Rvolid && \
				 (a).Rslot  == (b).Rslot)
#define		RIDCLEAR(a)	(a).Rpage = -1; (a).Rvolid = -1; (a).Rslot = -1
#define		TESTRIDCLEAR(a)	((a).Rpage == -1 && (a).Rvolid == -1 && (a).Rslot == -1)

/* extract the PID part form a RID */
#define	GETPID(p,r)	{ (p).Ppage = (r).Rpage; (p).Pvolid = (r).Rvolid; }

/* PRINTPID, PRINTFID, PRINTRID print PIDs, RIDs, FIDs for trace/debug */
/* Also includes pointer versions. */
#define	PRINTPID(a)	printf("{Pvolid=%d, Ppage=%d}", (a).Pvolid, (a).Ppage)
#define	PRINTFID(a)	printf("{Fvolid=%d, Ffilenum=%d}",\
					(a).Fvolid, (a).Ffilenum)
#define	PRINTRID(a)	printf("{Rvolid=%d, Rpage=%d, Rslot=%d}",\
					(a).Rvolid, (a).Rpage, (a).Rslot)
#define	PRINTPIDPTR(a)	if ((a)==NULL) {printf("NULL");} else { PRINTPID(*(a));}
#define	PRINTFIDPTR(a)	if ((a)==NULL) {printf("NULL");} else { PRINTFID(*(a));}
#define	PRINTRIDPTR(a)	if ((a)==NULL) {printf("NULL");} else { PRINTRID(*(a));}

/* Here's some of the stuff used to control buffers and files */
/*
	#define MAXUSERS 	4	maximum number of users allowed 
*/

#define	READ		0	/* file opened in READ mode */
#define	WRITE		1	/* file opened in WRITE mode*/

#define	HIGH		2	/* highest level hint */
#define	MID		1	/* middle level hint */
#define	LOW		0	/* lowest level hint */
#define	MAXHINT		3	/* the maximun possible value of hints */
#define	MINHINT		0	/* the minimun possible value of hints */

/* And here's some regular defs */
#define	FALSE		0
#define	TRUE		1
#define	SYNCH		0		/* for synchronous I/O */
#define	ASYNCH		1		/* for asynchronous I/O */

/* positions in a member list */
#define	FIRST		0
#define	LAST		-1
#define	PREV		0
#define	NEXT		-1

/* Where to insert in am_insertscan() */
#define	BEFORE		0
#define	AFTER		1	

	/* machine constants */
#define	BYTESPERWORD	((int)sizeof(int))
#define	BITSPERBYTE	8
#define	BITSPERWORD	(BITSPERBYTE*BYTESPERWORD)

	/* machine-dependent data types */
typedef int	FOUR;
typedef short	TWO;
typedef char	ONE;

	/* data structures used everywhere */

#define	TRACEMAPSIZE	BITSPERWORD
typedef int	TRACEFLAGS;

typedef struct 				/* record identifier */
	{
		FOUR	Rpage;
		FOUR	Rslot;
		TWO	Rvolid;
	} RID;

typedef struct				/* on-volume record identifier */
	{
		FOUR	SRpage;
		FOUR	SRslot;
	} SHORTRID;

typedef struct 				/* page identifier */
	{
		FOUR	Ppage;
		TWO	Pvolid;
	} PID;

typedef FOUR	SHORTPID;		/* on-volume page identifier */

typedef struct				/* file identifier */
	{
		TWO	Fvolid;
		TWO	Ffilenum;
	} FID;

typedef TWO	SHORTFID;		/* on-volume file identifier */

/* index cursor - used during a index scan */
typedef	struct
	{
		PID	pageid;		/* page on which the index is on */
		TWO	slotnum;	/* slot # of the key of this index */
		TWO	offset;		/* offset into the RID list */
	} XCURSOR;

#define	NULLPAGE	-1


/* data types exist on WiSS */
enum	data_type	{TNOATTR, TINTEGER, TLONG, TSHORT, TFLOAT, 
				 TDOUBLE, TSTRING, TBITS, 
					TUINTEGER, TULONG, TUSHORT}; /* YOUNG */

/* comparison operators used in Boolean expressions */
enum	rel_op		{EQ, NE, LT, LE, GT, GE, GE_LE};

/* logical operators used in Boolean expressions  Inserted by LSM */
enum	logical_op	{AND, OR, NOT, NONE};

typedef	struct	/* field descriptor */
{
	TWO		offset;		/* field offset into record */
	TWO		length;		/* field length */
	enum data_type 	type;		/* data type of the field */
} FIELDDESC;

typedef	struct	/* data descriptor */
{
	enum data_type 	type;		/* type of the data */
	char		value[MAXFIELD];/* value the data */
		/* Don't change the position in SUN4 :: CHANG */
	TWO		length;		/* length of the data */
} DATADESC;

typedef	struct /* boolean expression */
{
	enum rel_op	roperator;	/* relational operator */
	FIELDDESC	fielddesc;	/* field to be compared with */
	int		next;		/* pointer to next term */
	char		value[MAXFIELD];/* value to be compared with */
} BOOLEXP;

typedef FIELDDESC	KEYINFO;
typedef	DATADESC	KEY;

extern	FID NullFID;

#define	PRINTATTR(attr)	\
	printf("{type=%d, length=%d, offset=%d} ", \
		(int)(attr)->type, (attr)->length, (attr)->offset)
#define	PRINTKEY(key) \
	 if (key != NULL) print_data((key)->type,(key)->length,(key)->value);\
	 else printf(" empty ")

#define	MIN(x,y)	((x) < (y) ? (x) : (y))
#define	MAX(x,y)	((x) > (y) ? (x) : (y))

