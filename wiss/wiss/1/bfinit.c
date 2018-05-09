
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
/* Module bf_init:
	This routine initializes all the level 1 tables.

   Imports :
	BF_hashinit

   Exports :	
	bf_init()

   Errors :
	None

   Returns :
	None
*/

#include <bf.h>

extern char * shmAlloc();

/*
#include <stats.h>
*/

/*
init_stats()
{
    int  counter;

	IO_INTERLEAVE = READ_TIME = WRITE_TIME = no_IO_READS = 0;
	no_IO_WRITES = no_BUFFER_HITS = 0;
	QUEUE_TIME = SEEK_TIME = TRANSFORM_TIME = 0;
    
    for (counter = 0; counter < 2000; counter ++) disk_hist[counter] = 0;
}
*/
  

bf_init()
{
	register	i;

#ifdef TRACE
	if (checkset (&Trace1, tINTERFACE))
		printf("bf_init()\n");
#endif

	BF_hashinit();
#ifdef EVENTS
	BF_initevent();
#endif
/*
	init_stats();
*/

	/* initialize the principal level 1 latches */
	InitLatch(&smPtr->lockTableLatch);
	InitLatch(&smPtr->bufTableLatch);
	InitLatch(&smPtr->freeHashLatch);

	return(eNOERROR);
}


/* This routine initializes the hash table 
*/
BF_hashinit()
{
    	register int 	i;
    	register LOCKBUCKET  *bucketPtr;	/* lock bucket ptr */
    	register HASHBUCKET  *hash_bucketPtr;	/* hash bucket ptr */

	smPtr->hashBucket = 
		(HASHBUCKET *) shmAlloc(smPtr->hash_size*sizeof(HASHBUCKET));
	smPtr->hashTable = 
		(HASHBUCKET *) shmAlloc(smPtr->hash_size*sizeof(HASHBUCKET));
	smPtr->hashTableLatches = 
		(LATCH *) shmAlloc(smPtr->hash_size*sizeof(LATCH));

    	/* Initialize the page latch hash table  */

    	for (i = 0; i < LOCKTABLESIZE; i++) {
		InitLatch(&smPtr->PageLatch[i]);
		smPtr->PageLocks[i].inUse = FALSE;
		smPtr->PageLocks[i].nextBucket = NULL;
    	}

	/* Initialize the hash table used to find pages in the buffer pool */
    	for (i = 0; i < smPtr->hash_size; i++) {
		smPtr->hashTable[i].bufindex = NIL; /* not in use */
		smPtr->hashTable[i].nextBucket = NULL; /* not in use */
		InitLatch(&smPtr->hashTableLatches[i]);
    	}

    	/*  Build the list of free lock buckets */

    	smPtr->FreehashBucket = &smPtr->hashBucket[0];
    	hash_bucketPtr = &smPtr->hashBucket[0];
		for (i = 0; i < smPtr->hash_size - 1; i++, hash_bucketPtr++) {
		hash_bucketPtr->nextBucket = hash_bucketPtr + 1;
    	}
    	hash_bucketPtr->nextBucket = NULL;

    	smPtr->FreelockBucket = &smPtr->lockBucket[0];
    	bucketPtr = &smPtr->lockBucket[0];
    	for (i = 0; i < MAXLOCKBUCKET - 1; i++, bucketPtr++) {
		bucketPtr->nextBucket = bucketPtr + 1;
    	}
    	bucketPtr->nextBucket = NULL;
}

