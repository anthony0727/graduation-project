
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


#define MAX_TRANS_LOG    6
#define MAXTRANS  (1 << MAX_TRANS_LOG)     /* The maximum size of hash table */
#define lockhash(trans_id)   (trans_id & (MAXTRANS - 1))
	    /* The hash function for finding location of a given transaction */

#define SMALL_TRANS 0	/* transaction types : by size of lock requests */
#define MED_TRANS   1
#define BIG_TRANS   2

/* flink and blink link together all locks held by a transaction */
/* collision is used for the collision chain in the per transaction */
/* hash table */

struct lockq {
    struct   lockq	*flink, *blink; 
    struct   node       *resptr;
    struct   lockq	*collision;
    LOCKTYPE 		request;
};

/* to reduce the cost of testing whether a transaction
is already holding a lock on a particular page, two additional
hash tables are maintained: one for files and one for pages.
NOTE!! These hash tables do not need to reside in shared memory */

#define FIDLOCKTBLSIZE 47
#define PIDLOCKTBLSIZE 3597

struct fidLocalHash {struct lockq *fidChain[FIDLOCKTBLSIZE];};

struct pidLocalHash {struct lockq *pidChain[PIDLOCKTBLSIZE];};

#define HASHLOCALPID(_pid) ((_pid.Pvolid + _pid.Ppage ) % PIDLOCKTBLSIZE)
#define HASHLOCALFID(_fid) ((_fid.Fvolid + _fid.Ffilenum) % FIDLOCKTBLSIZE)


/*
 *  Each node of the TWFG is represented by graph_bucket.  Each node has a link
 *  representing which transaction it is waiting for.  It also has a link (resource
 *  wait) which corresponds to what resource it is waiting for.  This link
 *  is usefull since when a transaction is aborted you can directly remove it 
 *  from that resource's waiting list. Each cell also has a linked list of 
 *  resources which it allocated during the course of it's existance.
 */
struct graph_bucket {
    short    someone_waiting; /* Boolean value to indicate if a xact is waiting */
    int      trans_id;       /* The transaction this resource is waiting on */
    struct   graph_bucket    *waiting_for, *flink, *blink;
    LATCH    concurrent;
    struct   node            *res_wait;
    struct   wait_node       *resource_wait; /* What resource is the XACT 
			     waiting for */
    struct   lockq           *locks_held;
    struct   fidLocalHash    *fidHashTbl;
    struct   pidLocalHash    *pidHashTbl;
    short    aborted;
    int	     request;  /* one of the 6 lock mode types */
    int      no_pagelocks, no_pagereleases;
    int      no_filelocks, no_filereleases;
    int      no_blocks;
    int      no_upgrades;
};


/*
 *  The transaction table is implemented by a hash table on transaction id 
 *  The nodes corresponding to different transaction may form a linked list on
 *  a particular entry of table.  A latch is used to coordinate concurrent
 *  operations on this table 
 */
struct trans_bucket {
    LATCH	concurrent;
    struct   graph_bucket   *transptr;
};


