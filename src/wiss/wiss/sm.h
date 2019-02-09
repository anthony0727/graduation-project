
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


#include 	<latch.h>
#include	<st_h.h>
#include 	<st_d.h>
#include	<filed.h>
#include	<locktype.h>
#include 	<resource.h>
#include	<lockdefs.h>
#include	<graph.h>
#include	<stdatomic.h>
#include	<sys/types.h>


/* include file for variables and structures residing in shared memory */

#define LOCKTABLESIZE   179  /* hash table size for buffer pool page locks */
#define MAXLOCKBUCKET   179 * 2  /* max number of lock buckets */

#define	ALIGNTYPE	long long int

#ifdef EVENTS
#define MAXEVENTS    2500
#endif


/* following file is used by ftok(2) to control shared memory access */
#define SHM_KEYFILE     "/tmp/wissshmem"

/* definitions for controlling access to the latches */
#define IDLE 0
#define WANTIN 1
#define INCS 2

/* To simplify maintenance, a volume header is divided into 4 areas: 
   the main control header (which also includes the extent map), 
   the page map (one for each extent), the extent link and the 
   file descriptor area. Whenever any piece of information within an 
   area is modified, the entire area has to be updated (flushed to disk).
   In the current implementation, each header area of an active volume 
   is stored in a contiguious memory buffer.
*/

#define DEVNAMEMAX      80      /* max length of a device's physical name */
#define	NOVOL		-1	/* illegal Volume ID */

typedef struct 
{
	char	VDvolname [DEVNAMEMAX];	/* physical name */
	int	VDvolid;		/* Volume ID & phys addr */
	TWO	VDnumext;		/* # of extents on the volume */
	TWO	VDextsize;		/* # of pages per extent */
	int	VDnumpage;		/* total # of pages on the volume */
	int	VDheadersize[4];	/* # of blocks in each header area */
	PAGE	*VDheader[4];		/* buffer of each header area */
	ONE	VDdirty;		/* tells which pages are dirty */
	LATCH	latch;
} VolInfo;


typedef struct	
{       
	FOUR	Bfilenum;   /* open file number */
	FID	Bfid;	    /* used for cache invalidation */
	FOUR	transId;    /* used for cache invalidation also */
	PID	Bpageid;    /* the ID of the page in this buffer  */
	atomic_short	Bfixcnt;    /* # of active users */
	short	busy;
 	unsigned Bdirty:1;  /* has this page been modified ? */
	unsigned Brefbit:1; /* has buffer been referenced recently  */
	unsigned Bvalid:1;  /* dose this buffer contain a valid page  */
	LATCH	latch;
} BUFTAB; 

/* buffer pool latch table entry */

typedef struct lockbucket {
    short  lockCnt;	/* how many times page was locked */
    short  inUse;	/* TRUE or FALSE */
    LATCH  latch;	/* latch for the chain of all waiting processes */
    PID	   pid;		/* the page's pid */
    struct lockbucket *nextBucket;	/* next bucket in the chain */
} LOCKBUCKET;

typedef struct hashbucket {
	int 	bufindex;  /* index into buftable*/
	struct  hashbucket *nextBucket;	/* ptr to next bucket in linked list */
} HASHBUCKET;

union heapWord
{       union heapWord      *ptr;   /* pointer to the next block */
	ALIGNTYPE       dummy;  /* allignment unit */
	long long int             calloc; /* calloc clears integers */
};

typedef struct process {
	int	pid;	/* process id of the process */
	int	transId;/* current transaction id of process */
	SEMNODE	sem;
	SEMNODE	sems[MAXTHREADS];
} ACTIVEPROC;

#ifdef EVENTS
struct evententry
{
	int	process;
	char    event[10];
	PID     pageid;
	int     entry;
};
#endif



/* the following structure is used to describe the shared variables */
/* and data structures that reside in shared memory */

typedef struct {

    ACTIVEPROC	   users[MAXPROCS]; /* 1 for each connected process */
    int		   userCnt;	/* number of active users */

    /* first the global variables for managing the buffer pool itself */
    LATCH	    bufTableLatch; 
    int		    clock_hand;	 /* hand for clock algorithm - BFallocbuf.c */
    atomic_int		    bf_num_free; /* number of free buffers */
    int		    bf_num_bufs; /* total number of buffers in pool */
    BUFTAB   	    *buftable;   /* buffer control table */
    PAGE	    *bufferpool; /* actual buffer pool */

    /* the next set of variables deal with the hash table that is used */
    /* to determine if a particular page is in the buffer pool */

    int	hash_size;	/* hash table size */

    HASHBUCKET  *hashBucket; /* pool of hash buckets */
    HASHBUCKET  *hashTable;  /* the hash table - actually a malloced array */
    LATCH	*hashTableLatches; /* latches for hash table entries */
    HASHBUCKET *FreehashBucket;      /* list of free lock buckets */
    LATCH       freeHashLatch; 

    /* the next set of variables deal with the latch table that is used */
    /* to make sure a process does not read a page while another process */
    /* is reading or writing it from disk.  these variables have nothing */
    /* to do with the locktable actually used for locking pages and files. */

    LATCH	   lockTableLatch;
    LATCH	   PageLatch[LOCKTABLESIZE]; /* array of Latches */
    LOCKBUCKET	   PageLocks[LOCKTABLESIZE]; /* array of Page Locks */
    LOCKBUCKET     lockBucket[MAXLOCKBUCKET]; /* pool of free lock buckets */
    LOCKBUCKET     *FreelockBucket;   /* list of free lock buckets */

    /* the following table is used to trace buffer pool events */
    /* it should be deleted in a production system */
    /* EVENTS is currently not defined */
#ifdef EVENTS
    struct  	evententry      event_table[MAXEVENTS];
    int     	eventhead;
    LATCH	eventLatch;
#endif

    /* the following declarations deal with level 0 data structures */

    LATCH 	VolDevLatch;    /* latch  for volume mounts & dismounts */
    VolInfo	VolDev[MAXVOLS];  /* information on mounted volumes */

    /* the following declarations deal with level 2 data structures */
    LATCH	level2Latch;
    /* volume table for info on directories of mounted volumes */
    DIRTBLENTRY	dirtable[MAXVOLS];
    int		volMountCnt[MAXVOLS]; /* number of level 2 mounts of same vol */

    /* tables for information on open files */
    filetable_s filetable[MAXOPENFILES];
    files_s	files[MAXOPENFILES];

    /* shared data structures for lock manager */
    int	   transCounter;  /* counter for assigning unique transaction #s*/

    struct lockq freelocks [MAXNODE];  /* list of free locks */
    LATCH	freeLockLatch; /* assures integrity of the freelocks list */
    struct node	freenodes[MAXNODE]; /* The free list of nodes */
    LATCH	freeNodeLatch; /* assures integrity of the freenodes list */
    struct graph_bucket freegraph[MAXTRANS]; /* list of free graph_buckets */
    struct wait_node freewaitnodes[MAXWAITNODE]; /* free list of wait nodes*/
    LATCH	lockLatch; /* assures integrity of the freegraph and */
			/* freewaitnodes lists */

    /* The Wait-For-Graph - one entry per active transaction */
    struct    trans_bucket    trans[MAXTRANS];
    /* the lock table containing resources - one entry for each page
    	or file lock that has been set*/
    struct  locknode     locktable[MAXRES]; 

    LATCH	wfg;	/* used for locking the whole waitforgraph when */
    			/* deadlock detection is taking place */

    /* the following are used for statistics gathering purposes within
    	the lock manager */
    int     locks_req;     /*    number of locks requested */
    int     no_deadlocks;  /*    number of deadlocks occured */
    int	    no_committs;   /*    number of transaction commits */
    int     no_aborts;     /*    number of aborts occuring in the system */
    int     no_blocks;     /*    total number of blocks induced by the system */

    /* following declarations are used for shmalloc calls (in wiss/util) */
    union heapWord *search_ptr; /* search ptr */
    union heapWord *heap_top;   /* heap top */
    size_t		   heapSize;	/* size of the heap in heapWord units */
    /* the heap begins below.  Since we don't know the actual size */
    /* until run time, we just initialize the size to be 1 for now*/
    union heapWord heap[1];     

    /* NOTE. nothing can go below here!!!!!!! */
} SMDESC; 

