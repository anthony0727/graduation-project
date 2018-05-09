
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


#define DATAFREE(p)  ((int)sizeof(DATAPAGE)-DPFIXED-\
			(p)->free-(p)->ridcnt*(int)sizeof((p)->slot[0]))
/* kinds of level 2 records */
#define	NORMAL		0
#define	SLICE		1
#define	CRUMB		2

#define	HEADERLEN	(2*(int)sizeof(ONE)+(int)sizeof(FOUR))

/* types of level 2 records */
#define	MOVED		0
#define NOTMOVED	1
#define NEWHOME		2

/* structure of level two records */
typedef struct {
		ONE	type;			/* type of record */
		ONE	kind;			/* kind of record */
		FOUR	length;			/* number of data bytes */
		char	data[1];		/* start of data area */
} RECORD;


