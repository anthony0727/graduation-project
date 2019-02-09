
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
/*  Module BF_hash:
 	The routines in this module maintain a hash table
	for assoicative accesses to the buffer table.

   Imports :	
	buftable[] 

   Exports :
	int BF_lookup(pageid)
	BF_insert(pageid, ix)
	BF_delete(pageid, flag)
	BF_lock(pageid)
	BF_unlock(pageid)

*/

#include	<bf.h>
#include	<stats.h>

/*
	extern int bufferHits;
	extern int bufferRefs;
*/

#define Hash(pid) 					\
            ((pid->Ppage + pid->Pvolid) % smPtr->hash_size)

#define BF_Hashlock(pid) 					\
            ((pid.Ppage + pid.Pvolid) % LOCKTABLESIZE)


int
BF_lookup(pageid)
PID	*pageid;
{
	register int	tableIdx;
	register HASHBUCKET	*bucketPtr;
	PID	locPid;


#ifdef	TRACE
	if (checkset (&Trace1, tHASHTAB))
		printf("BF_lookup(pageid=%d:%d)\n",
			pageid->Pvolid, pageid->Ppage);
#endif

/*
	bufferRefs++;
*/
	tableIdx = Hash(pageid);
	/* first set the appropriate latch */
	SetLatch (&smPtr->hashTableLatches[tableIdx], procNum, NULL);

	bucketPtr = &smPtr->hashTable[tableIdx];
        if ((bucketPtr->bufindex == NIL) && (bucketPtr->nextBucket==NULL))
	{
	    /* there are no other entries in this chain so the page
	    must not be in the hash table, release the latch and return  */
#ifdef EVENTS 
	    BF_event(procNum,"BFlookup",pageid, NIL);  
#endif
	    ReleaseLatch(&smPtr->hashTableLatches[tableIdx], procNum);
	    return(NIL);
	}
	else
	{
	    locPid = *pageid;
	    while (bucketPtr != NULL)
	    {
		if (PIDEQ(smPtr->buftable[bucketPtr->bufindex].Bpageid, locPid))
		{
/*
		    bufferHits++;
*/
		    ReleaseLatch(&smPtr->hashTableLatches[tableIdx],procNum);
#ifdef EVENTS 
                    BF_event(procNum,"BFlookup",pageid,bucketPtr->bufindex);  
#endif
		    return(bucketPtr->bufindex);
		}
		bucketPtr = bucketPtr->nextBucket;
	}

	/*at this point, we know that the page was not found */
#ifdef EVENTS 
	BF_event(procNum,"BFlookup",pageid,-1);  
#endif
	ReleaseLatch(&smPtr->hashTableLatches[tableIdx], procNum);
	return(NIL);
    }
}


/* Insert a new entry into the hash table */
BF_insert(pageid, ix)
PID	*pageid;	/* page ID as the hash key */
int	ix;		/* where to insert the new entry */
{
	register HASHBUCKET *bucketPtr;
    	register HASHBUCKET *newBucketPtr;  
    	register int tableIdx;			  /* hash table index */

#ifdef	TRACE
	if (checkset (&Trace1, tHASHTAB))
		printf("BF_insert(pageid=%d:%d, index=%d)\n",
			pageid->Pvolid, pageid->Ppage, ix);
#endif



    tableIdx = Hash(pageid);  /* compute location of page in hash table */

    /* set the appropriate latch */
    SetLatch (&smPtr->hashTableLatches[tableIdx], procNum, NULL);
#ifdef EVENTS 
    BF_event(procNum,"BFinsert",pageid,ix);  
#endif

    /* try and see if we can avoid going to the free list */
    bucketPtr = &smPtr->hashTable[tableIdx];
    if (bucketPtr->bufindex == NIL)
    {
	/* primary hash entry is free, use it */
	bucketPtr->bufindex = ix;
        ReleaseLatch (&smPtr->hashTableLatches[tableIdx], procNum);
	return(eNOERROR);
    }
    else
    {
	/* primary entry was in use, must allocate a free bucket */
        /* set a latch to prevent concurrent access to the free list */
        SetLatch(&smPtr->freeHashLatch, procNum, NULL);
        if (smPtr->FreehashBucket == NULL ) 
	{
	    ReleaseLatch(&smPtr->hashTableLatches[tableIdx], procNum);
	    ReleaseLatch(&smPtr->freeHashLatch, procNum);
            printf("BF_insert: error, out of hash buckets!!!\n");
	    BF_dumpbuftable();
	    BF_dumpfixed();
	    return(eNOMOREBUCKETS);
        }
        /* Allocate and intialize a bucket */
        newBucketPtr = smPtr->FreehashBucket;
        smPtr->FreehashBucket = smPtr->FreehashBucket->nextBucket;

	/* attach the new bucket to the list of buckets on this chain */
        newBucketPtr->nextBucket = bucketPtr->nextBucket;
	bucketPtr->nextBucket = newBucketPtr;
        newBucketPtr->bufindex = ix;

	/* release the latches and then return */
        ReleaseLatch(&smPtr->freeHashLatch, procNum);
        ReleaseLatch(&smPtr->hashTableLatches[tableIdx], procNum);
        return(eNOERROR);
    }
}


BF_delete(pageid)
PID	*pageid;

{
    register HASHBUCKET  *bucketPtr, *prevBucket, *nextBucket;  /* bucket pointer */
    register int tableIdx;		     	/* hash table index */
    PID	locPid;


    tableIdx = Hash(pageid); /* compute location of page in hash table */

    /* set the appropriate latch */
    SetLatch (&smPtr->hashTableLatches[tableIdx], procNum, NULL);
    bucketPtr = &smPtr->hashTable[tableIdx];

    /* first handle the normal case that the page is in the primary bucket */
    locPid = *pageid;
    if (PIDEQ(smPtr->buftable[bucketPtr->bufindex].Bpageid, locPid)) 
    {
	/* page was in the primary bucket */
	/* simply mark free and return */
#ifdef EVENTS 
	BF_event(procNum,"BFdelete1", pageid, bucketPtr->bufindex);   
#endif
	bucketPtr->bufindex = NIL;

	//james 161007
	if (bucketPtr->nextBucket != NULL)	{
		nextBucket = bucketPtr->nextBucket;

		*bucketPtr = *nextBucket;

		/* set a latch to prevent concurrent access to the free list */
		SetLatch(&smPtr->freeHashLatch, procNum, NULL);
		/* and then add it back onto the free list */
		nextBucket->nextBucket = smPtr->FreehashBucket;
		smPtr->FreehashBucket = nextBucket;

		ReleaseLatch(&smPtr->freeHashLatch, procNum);
	}
	//james 161007
        ReleaseLatch (&smPtr->hashTableLatches[tableIdx], procNum);
	return(eNOERROR);
    }
    else
    {
	/* page was not in the primary bucket, must loop 
	until we find it */

	prevBucket = bucketPtr;
	bucketPtr = bucketPtr->nextBucket;
        while (bucketPtr != NULL) 
        {
	    if (PIDEQ(smPtr->buftable[bucketPtr->bufindex].Bpageid, locPid)) 
	    {
    		/* set a latch to prevent concurrent access to the free list */
    		SetLatch(&smPtr->freeHashLatch, procNum, NULL);
		/* next unlink the bucket to be removed from the chain */
		prevBucket->nextBucket = bucketPtr->nextBucket;
		/* and then add it back onto the free list */
		bucketPtr->nextBucket = smPtr->FreehashBucket;
		smPtr->FreehashBucket = bucketPtr;

#ifdef EVENTS 
		BF_event(procNum,"BFdelete2", pageid, bucketPtr->bufindex);   
#endif
		ReleaseLatch(&smPtr->freeHashLatch, procNum);
    		ReleaseLatch(&smPtr->hashTableLatches[tableIdx], procNum);
		return(eNOERROR);
            }
	    prevBucket = bucketPtr;
	    bucketPtr = bucketPtr->nextBucket;
	}
    }
    printf("BF_delete: error, no entry exists for page (%d, %d) tableIdx=%d\n",
	pageid->Pvolid, pageid->Ppage, tableIdx);
    BF_hash_dump();
    BF_dumpbuftable();
#ifdef EVENTS
    BF_dumpevent();
#endif
    ReleaseLatch(&smPtr->hashTableLatches[tableIdx], procNum);
    return(eNOENTRY);
}

/*     The following routines are used to lock pages
 *	Page locks are a restricted form of general locks where
 *	the only lock mode is EXCLUSIVE.
 *	Because of this restriction, we don't have to keep lock lists
 *	for page locks; all we need to do is keep a count of the
 *	number of times the page was locked.
 *
 * IMPORTS:
 *	semaphore routines.
 *
 * EXPORTS:	
 *
 *	Page locks:
 *		BF_Lock (pid)
 *		BF_Unlock (pid)
 *
 */


/*============================================================================*/
/* 
 * BF_lock
 * 	Sets an Exclusive latch on a page in the buffer pool
 *
 * RETURNS:
 *	eNOERROR if all went well.
 *
 * ERROR RETURNS:
 *	e1BADPARAMS - if there were bad input parameters.
 *
 * SIDE EFFECTS:
 *      If another process already has the page latched, the
 *      caller is blocked until the page is unlatched (via
 *	a call to BF_unlock)
 */

int
BF_lock (pid)
PID *pid;		/* the page to be locked */
{
    register LOCKBUCKET  *bucketPtr, *newBucket;    /* bucket pointer */
    register int tableIdx;			    /* hash table index */
    PID locPid ;

/*
printf("BF_Lock(pid.Pvolid = %d, pid.Ppage = %d)\n", pid->Pvolid, pid->Ppage);
*/


    /*
     * Hash the pid and see if it's in the hash.
     * Remember the bucket pointer we took in prevNextBucket
     * in case we have to update the bucket list.
     */

    locPid = *pid;
    tableIdx = BF_Hashlock(locPid);

    /* get a latch on the tableIdx entry of the PageLocks array by
    setting the corresponding PageLatch entry. This was done this way
    to avoid the use of a global buffer pool semaphore */
    SetLatch(&smPtr->PageLatch[tableIdx], procNum, NULL);

#ifdef EVENTS 
    BF_event(procNum,"BF_lockR",&locPid,-1); 
#endif

    bucketPtr = &smPtr->PageLocks[tableIdx];
    if ((bucketPtr->inUse == FALSE) && (bucketPtr->nextBucket==NULL))
    {
	/* nobody else in this chain of LOCKBUCKETS so we can just
	set our lock safely and return */
        bucketPtr->lockCnt = 1;
        bucketPtr->pid = locPid;
	bucketPtr->inUse = TRUE;

        /* initialize and set the latch in the bucket so that any subsequent 
           processes will wait if they attempt to lock the same page.  */
        InitLatch(&bucketPtr->latch);
        SetLatch(&bucketPtr->latch, procNum, NULL);
        ReleaseLatch(&smPtr->PageLatch[tableIdx], procNum);
#ifdef EVENTS 
	BF_event(procNum,"BFlockG1",&locPid,-1); 
#endif
        return(eNOERROR);
    }
    else
    {
	/* somebody else has a page locked on this chain */
	/* see if it is a conflicting lock first */

        while (bucketPtr != NULL) 
	{
	    if (PIDEQ(bucketPtr->pid, locPid)) 
	    {
	        /*
	         * The lock was in the hash.
	         * Make the caller wait.
	         */
	        bucketPtr->lockCnt++;

	        /* in the process of setting bucketPtr->latch, the
	          SetLatch() call will safely release the latch on the whole 
	          chain just before putting the process to sleep by making it
	          wait on its semaphore */
	       SetLatch(&bucketPtr->latch, procNum,&smPtr->PageLatch[tableIdx]);
#ifdef EVENTS 
	       BF_event(procNum,"BFlockG2",&locPid,-1); 
#endif
	       return(eNOERROR);
	    }

	    /*
	     * The lock didn't match that bucket, look
	     * at the next bucket.
	     */
	    bucketPtr = bucketPtr->nextBucket;
	}

        /*
         * If we got here, the lock wasn't in the hash.
         * First see if we can avoid going to the free list 
	 * for a lockbucket *.
         */
         bucketPtr = &smPtr->PageLocks[tableIdx];
         if (bucketPtr->inUse == TRUE)
	 {
	      /* need to get a free lockbucket */
              /* Begin by setting a latch on the free list */
	      /* we panic if we are out of lockbuckets.  a better */
	      /* solution would be to go and malloc more lock */
	      /* buckets but we have never seen that happen */

              SetLatch(&smPtr->lockTableLatch, procNum, NULL);
              if (smPtr->FreelockBucket == NULL ) 
	      {
                  printf("BF_Lock: error, out of lock buckets, pid %d!!!\n",
			  pid->Ppage);
                  ReleaseLatch(&smPtr->lockTableLatch, procNum);
                  ReleaseLatch(&smPtr->PageLatch[tableIdx], procNum);
	          return(eNOMOREBUCKETS);
               }
               /* allocate a lock bucket  */
               newBucket = smPtr->FreelockBucket;
               smPtr->FreelockBucket = smPtr->FreelockBucket->nextBucket;
               ReleaseLatch(&smPtr->lockTableLatch, procNum);

	       /* connect bucket to the chain */
	       newBucket->nextBucket = bucketPtr->nextBucket;
	       bucketPtr->nextBucket = newBucket;
	       bucketPtr = newBucket;
	 }

	 /* at this point bucketPtr is pointing at the bucket to use */
     	 bucketPtr->lockCnt = 1;
    	 bucketPtr->pid = locPid;
	 bucketPtr->inUse = TRUE;

         /* initialize and set the latch in the bucket so that any subsequent 
            processes will wait if they attempt to lock the same page.  There
            is no chance of waiting here so we don't need to release the
            lockTableLatch */
         InitLatch(&bucketPtr->latch);
         SetLatch(&bucketPtr->latch, procNum, NULL);
         ReleaseLatch(&smPtr->PageLatch[tableIdx], procNum);
#ifdef EVENTS 
         BF_event(procNum,"BFlockG3",&locPid,-1); 
#endif
         return(eNOERROR);
    }
}


/*============================================================================*/
/* 
 * BF_unlock
 * 	Release a page latch.
 *
 * RETURNS:
 *	eNOERROR if all went well.
 *
 * ERROR RETURNS:
 *	e1BADPARAMS  - if there was a bad input paramter.
 *	e1LOCKERROR  - if no latch existed for the page.
 *
 * SIDE EFFECTS:
 *	The page is  unlatched.
 *	A waiting process may be awakened.
 */

int
BF_unlock (pid)
PID	*pid;
{
    register LOCKBUCKET  *bucketPtr, *headPtr;  /* bucket pointer */
    register LOCKBUCKET *prevBucket;    	/* ptr to prev nextBucket ptr */
    register int tableIdx;			     	/* hash table index */
    PID	locPid ;

/*
	printf("BF\nBF_Unlock(pid.Pvolid = %d, pid.Ppage = %d)\n", 
	pid->Pvolid, pid->Ppage);
*/

    locPid = *pid;

#ifdef DEBUG
    if (pid == NULL) {
	printf("BF_Unlock: error, NULL pid parameter\n");
	return(e1BADPARAMS);
    }
#endif

    /*
     * Hash the pid and see if it's in the hash.
     */
    tableIdx = BF_Hashlock(locPid);
    SetLatch(&smPtr->PageLatch[tableIdx], procNum, NULL);

#ifdef EVENTS 
    BF_event(procNum,"BFunlock",&locPid,-1); 
#endif

    bucketPtr = &smPtr->PageLocks[tableIdx];
    headPtr = bucketPtr;
    prevBucket = bucketPtr;
    while (bucketPtr != NULL) 
    {
	if ((bucketPtr->inUse == TRUE) && (PIDEQ(bucketPtr->pid, locPid)))
	{
	    /*
	     * Found the correct latch 
	     * Decrement the count reflecting the number of
	     * times BF_Lock() on this page was called.
	     * If nobody else is waiting trying to lock the page,
	     * which is detected when lockCnt equals 0,
	     * then the bucket can be freed.
	     */
	    bucketPtr->lockCnt--;
	    if (bucketPtr->lockCnt == 0) 
	    {
		/* nobody is waiting.  Even though nobody is waiting we */
		/* release the latch */
		/* this step could probably be eliminated but */
		/* was left in so that we could count that the number */
		/* of SetLatch and ReleaseLatch calls was the same */
		ReleaseLatch (&bucketPtr->latch, procNum);
		bucketPtr->inUse = FALSE; /* mark the bucket free */
		bucketPtr->pid.Ppage = -1; /* to insure nobody finds it */
		if (bucketPtr != headPtr)
		{
		    /* bucket was not head bucket, must return it to the 
		    free list. If it was the head bucket we can simply do
		    nothing. Start by acquiring a latch on the free list */
              	    SetLatch(&smPtr->lockTableLatch, procNum, NULL);
		    /* then unlink the bucket from the chain */
		    prevBucket->nextBucket = bucketPtr->nextBucket;
		    /* and add it back onto the free list */
		    bucketPtr->nextBucket =  smPtr->FreelockBucket;
		    smPtr->FreelockBucket = bucketPtr;
		    /* finally, release the free list latch */
               	    ReleaseLatch(&smPtr->lockTableLatch, procNum);
		}
	    }
	    else
	    {
		/* somebody was waiting */
#ifdef DEBUG
	    	printf("BF_Unlock: waking waiter\n");
#endif
#ifdef EVENTS 
	    	BF_event(procNum,"BFunlockS",&locPid,1); 
#endif
	    	ReleaseLatch(&bucketPtr->latch, procNum);
	    }
    	    ReleaseLatch(&smPtr->PageLatch[tableIdx], procNum);
	    return(eNOERROR);
	}

	/*
	 * The lock didn't match that bucket, look
	 * at the next bucket.
	 */
	prevBucket = bucketPtr;
	bucketPtr = bucketPtr->nextBucket;
    }

#ifdef EVENTS 
	BF_event(procNum,"unlock_fail",&locPid,2); 
#endif

    printf ("BF_unlock error, no latch on page (%d, %d).\n",
	pid->Pvolid,pid->Ppage);
    ReleaseLatch(&smPtr->PageLatch[tableIdx], procNum);
    BF_lock_dump();
#ifdef EVENTS 
    BF_dumpevent();
#endif
    return(eNOENTRY);
}

BF_lock_dump()
{
	int	i;
	LOCKBUCKET *l;
	
	printf("Lock Table\n");
	printf("index\tpage\tlockCnt\tinUse\tnextBucket\n");
	for (i = 0; i < LOCKTABLESIZE; i ++){
		l = &smPtr->PageLocks[i];
		while (l!=NULL){
			printf (" %d\t %d\t%d\t%d\t%x--->  ",
			i,l->pid.Ppage,l->lockCnt, l->inUse, l->nextBucket);
			l=l->nextBucket;
			}
		printf("\n");
	}
	printf("*************************\n");
}
BF_hash_dump()
{
	int	i;
	HASHBUCKET *l;
	
	printf("Hash Table\n");
	printf("index\bufindex\tpid.page\n");
	for (i = 0; i < smPtr->hash_size; i ++){
		l = &smPtr->hashTable[i];
		if ((l->bufindex == NIL) && (l->nextBucket == NULL)) continue;
		while (l!=NULL){
			printf (" %d\t %d\t %d---->",i,l->bufindex,
				smPtr->buftable[l->bufindex].Bpageid.Ppage);
			l=l->nextBucket;
			}
		printf("\n");
	}
	printf("*************************\n");
}
