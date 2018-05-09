
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
#include	<bt_error.h>

/* definitions internal to level 2, B-tree routines */

/* trace flags used by btree routines */
#define tBTREE		4	/* BTREE interface routines */
#define	tBTREEUTIL	5	/* BTREE utilities interface */
#define	tCOMPRESS	6	/* trace of the page in bt_compress_page */
#define	tSEARCH		7	/* search on page trace */
#define	tINSERTENTRY	8	/* insert entry trace */
#define	tAPPENDRID	9	/* append rid trace */
#define	tMOVEENTRIES	10	/* move entries between pages */
#define	tTRAVERSE	11	/* traversal algorithm trace */
#define	tEXTRACTKEY	12	/* dump of temporary kye-rid file */
#define	tFILLLEAF	13	/* trace of fill leaf in st_createindex */
#define	tREORGANIZE	14	/* reorganization after index deletion */
#define	tINSERTKEY	15	/* trace of insert key */
#define	tTREEDUMP	16	/* verbose tree dump, used in conjection
				   with other trace flags */

/* constants for pagetype, and tree type */
#define	ROOTPG		'r'	/* root page */
#define	NODEPG		'n'	/* node page */
#define	LEAFPG		'l'	/* leaf page */
#define	OVFLPG		'o'	/* overflow page */

#define	INDEX		'I'	/* dense btree index */
#define	PRIMINDEX	'P'	/* primary btree index */
#define	LINK		'L'	/* link btree */
#define	VERSION		'V'	/* version btree */
#define	HASH		'H'	/* hash table */

/* char constants for suffix type (for keyrid and index files) */
#define	INDEXSUF	'i'	/* index file suffix */
#define	KEYRIDSUF	'k'	/* keyrid file suffix */
#define	TMPSUF		't'	/* temporary file suffix */
#define	HASHSUF		'h'	/* hash file suffix */

#define	MAXKEYLEN	256
#define	MAXRIDCOUNT	((int)(PAGESIZE-MAXKEYLEN)/(3*(int)sizeof(RID)))

/* Here's the header declaration for the root page of each B-tree */
typedef struct {
	KEYINFO		keyattr;
} INDEXHEADER;

/*
 	To simply moving back up a btree, the traversal path (of pids)
	is stored in an array of type PARENTLIST.  There is a maximum number
	of possible parents -- MAXPARENTS -- deeper trees require recompilation.
*/

#define	MAXPARENTS	10

typedef struct 
{
    PID	          page_id;   /* The page being referenced */	
    short         locked;    /* if the corresponding page is locked */
}PARENTLIST[MAXPARENTS];


typedef	TWO		PARENTINDEX;

/*     macros for dealing with B-tree entries			   */
#define MOVERIDCOUNT(to, from) 	((char *)(to))[0] = ((char *)(from))[0], \
		((char *)(to))[1] = ((char *)(from))[1];
#define	MOVERID(to, from)	movebytes((char *)(to), (char *)(from), (int)sizeof(RID))
#define	MOVESHORTPID(to, from)	movebytes((char *)(to), (char *)(from), (int)sizeof(SHORTPID))

/* find the amount of usable space in the free area of the page */
#define	USABLESPACE(p)	((int) (ENDDATA) - p->btcontrol.enddata\
			 - (p->btcontrol.numoffsets)*(int)sizeof(p->slot))
#define	MAXSPACE(p)	((ENDDATA) - p->btcontrol.startdata)
#define	USEDSPACE(p)	(MAXSPACE(p) - p->btcontrol.numfree)
#define	FREESPACE(p)	(p->btcontrol.numfree)

/* macros to access entries on a btree page */
#define	ENTADDR(p,s)	(&((p)->data[(p)->slot[-(s)]]))
#define	KEYLEN(p,s)	(*(TWO *)ENTADDR(p,s))
#define KEYVALUE(p,s)	(ENTADDR(p,s) + 2)

/* macros to access info in a key-ptr pair (on a node or root page) */
#define	GETPTR(p,s,ptr) \
	MOVESHORTPID((char *)&(ptr),ENTADDR(p,s)+2+KEYLEN(p,s))
#define	KEYPTRLEN(p,s)	((int) KEYLEN(p,s) + (int)sizeof(SHORTPID) + 2)
#define MAKEKEYPTR(buff, key_len, key, ptr, total_length)\
	{ TWO z_count; register char *z_s1, *z_s2; \
	 z_count = key_len; *(TWO *)(buff)= z_count; \
	 z_s1 = ((char *)(buff)) + 2; z_s2 = key;\
	 total_length = z_count + 2 + (int)sizeof(SHORTPID);\
	 for (; z_count > 0; z_count--) *(z_s1++) = *(z_s2++);\
	 MOVESHORTPID(z_s1,(&(ptr).Ppage)); }

/* macros to access info in a key-rid list (on a leaf page) */
#define	RIDCOUNT(p,s)	(ENTADDR(p,s) + KEYLEN(p,s) + 2)
#define	RIDLIST(p,s)	(RIDCOUNT(p,s)+2)
#define	KEYRIDLEN(p,s,len) \
	{ MOVERIDCOUNT((char *)&(len), RIDCOUNT(p,s));\
	  if (len < 0) len = 1;\
	  len = (len) * (int)sizeof(RID) + KEYLEN(p,s) + 4;}

#define MAKEKEYRID(buff, key_len, key, ridptr, total_length)\
	{ TWO z_count; register char *z_s1, *z_s2;\
	  z_count = key_len; *(TWO *)(buff) = z_count; \
	  z_s1 = ((char *)(buff)) + 2; z_s2 = key;\
	  total_length = z_count + 2 + 2 + (int)sizeof(RID);	\
	  for (; z_count > 0; z_count--) *(z_s1++) = *(z_s2++);\
	  z_count = 1; MOVERIDCOUNT(z_s1, (char *)&z_count);\
	  z_s1 += 2; MOVERID(z_s1, (char *)(ridptr)); }\

#define	ENTRYLEN(p, s, len)	\
	if (p->btcontrol.pagetype == 'l') KEYRIDLEN(p, s, len)\
	else (len) = KEYPTRLEN(p, s);

/* macros for debug printing */
#define	PRINTPARENTLIST(list, index) \
	{ int i; printf(" parent list :"); for (i=0; i <= index; i++)  \
			{ printf("%s", i ? ",":" "); PRINTPID(list[i]); } }

/* macros used for concurrency control */
#define OVERFLOW(p)    (p->btcontrol.numfree < MAXKEYLEN)
#define UNDERFLOW(p)   ( (USEDSPACE(p) - MAXKEYLEN) < MAXSPACE(p)/2)
