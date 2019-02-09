
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


/* debug flag code */
#define	tHASHFILE	1

/* max # of buckets; ie, max # of entries in the hash table */
#define	MAXBUCKETS	PAGESIZE / (int)sizeof(SHORTPID)
#define	MAXDEPTH	11	/* this should be LOG2(MAXBUCKETS) */

/* modified by PHJ
#define	MAXDEPTH	10	
*//* this should be LOG2(MAXBUCKETS) */


/* root (hash table) page structure */
typedef struct {
	SHORTPID 	bucket[MAXBUCKETS];
} ROOTPAGE;

#define	GLOBALDEPTH(f)	(F_FILESIZE(f))
#define	LOCALDEPTH(dp)	((dp)->btcontrol.pid0)
#define	BUDDY(i,depth)	(depth ? (i) ^ (1 << (depth - 1)) : 0)

