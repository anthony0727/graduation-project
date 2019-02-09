
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
#include        <wiss.h>
#include        <page.h>
#include        <sm.h>

extern  SMDESC  *smPtr;

InitLatch(latchPtr)
LATCH *latchPtr;
{
    latchPtr->busy = FALSE;
    latchPtr->semList = NULL;
#if defined(TESTANDSET)
    testandset_init(&latchPtr->sync);
#else
	atomic_flag_clear(&latchPtr->sync);
#endif
}

/* 
   set the latch pointed to by latchPtr.  In the process, if
   releaseLatchPtr is not NULL,  release the latch it points at.
   This is down at safe point and enables us to crab our way
   down a latch hierarchy safely whether we have to wait or not
*/


SetLatch(latchPtr, procNum, releaseLatchPtr) 
LATCH   *latchPtr;	/* latch being set */
int	procNum;	/* process identifier (0..MAXPROCS-1) */
LATCH   *releaseLatchPtr; /* latch to be released if not not NULL */
{
    int queuedSelf;	/* whether the process queued up its sem */
	int i;
    SEMNODE *semPtr, *privateSem;

    /* 
     * Grab the latch's gate.
    SpinLock(L.Gate);
     */

/*
    printf("proc %d, SL on latch:%x\n",procNum, latchPtr);
*/

#if defined(TESTANDSET)
    while (testandset(&latchPtr->sync, 1) != 0)
	;
#else
    /* the following code is taken from Peterson and Silberschatz pp 313 */
    /* on machines with hardware test and set instructions it should be */
    /* replaced with code that uses the test and set */
	
	//james 161007
	while (atomic_flag_test_and_set(&latchPtr->sync));
	//james 161007
#endif

    /* at this point we in the critical section alone */
/*
    printf("proc %d enters C.S. in SL\n",procNum);
*/

    /*
     * If the latch is busy (i.e., in use), put the process's 
     * private semaphore on the latch's queue. Don't block the 
     * process yet, though, as we still have to release the gate.
     */
    if (latchPtr->busy == TRUE) {
		//james 161010
		privateSem = NULL;
	queuedSelf = TRUE;
	/* now add the process's private sem to the list of waiting procs */
	/* first get a pointer to processor's private semaphore */

	while (privateSem == NULL)	{
		for (i = 0; i < MAXTHREADS; i++)	{
			if (!atomic_flag_test_and_set(&smPtr->users[procNum].sems[i].sync))	{
				privateSem = &smPtr->users[procNum].sems[i];
				break;
			}
		}
	}
	//james 161010
	privateSem->nextSem = NULL;
	if (latchPtr->semList == NULL) latchPtr->semList=privateSem;
	else
	{
	    /* another process was already waiting */
	    semPtr = latchPtr->semList;
	    while (semPtr->nextSem != NULL) {
		semPtr=semPtr->nextSem;
	    }
	    semPtr->nextSem = privateSem;
         }
    }
    else {
	queuedSelf = FALSE;
    }
    latchPtr->busy = TRUE;

    /*
     * Release the latch's gate. If the process queued its semaphore, 
     * it has to wait on its private semaphore. Note that there is
     * a race condition here, since the current owner of the latch
     * can sneak in and do a V(PrivateSem) before the P(PrivateSem) 
     * below is done. That's ok, though, since the operating system 
     * will remember the V().
     */
     /* first see if we are to release a latch */
      if (releaseLatchPtr != NULL) ReleaseLatch(releaseLatchPtr, procNum);

    /* end of the critical section - release the gate - SpinUnlock(L.gate) */
#if defined(TESTANDSET)
    testandset_release(&latchPtr->sync);
#else
	//james 161007
	atomic_flag_clear(&latchPtr->sync);
	//james 161007
#endif
/*
    printf("proc %d leaves C.S. in SL\n",procNum);
*/

    if (queuedSelf == TRUE) {
/*
	printf("p%d sleeps on L%x\n",procNum, latchPtr);
*/
	WaitSem(&privateSem->semid);
	//james 161010
	atomic_flag_clear(&privateSem->sync);
	//james 161010
    }
}

ReleaseLatch(latchPtr, procNum)
LATCH   *latchPtr;	/* latch being released */
int	procNum;	/* processor number */
{
    SEMNODE *semPtr, *privateSem;

/*
    printf("proc %d, RL on latch:%x\n",procNum, latchPtr);
*/
    /* 
     * Grab the latch's gate.  -  SpinLock(L.gate);
     */
#if defined(TESTANDSET)
    while (testandset(&latchPtr->sync, 1) != 0)
	;
#else
    /* the following code is taken from Peterson and Silberschatz pp 313 */
    /* on machines with hardware test and set instructions it should be */
    /* replaced with code that uses the test and set */

	//james 161007
	while (atomic_flag_test_and_set(&latchPtr->sync));
	//james 161007
#endif

    /* at this point we in the critical section alone */
/*
    printf("proc %d enters C.S. in RL\n",procNum);
*/

    /*
     * If no other process is waiting for the latch,
     * then mark the latch as free (i.e., not busy).
     * Else, dequeue the first waiter on the latch's queue.
     */
    if (latchPtr->semList == NULL) {
	latchPtr->busy = FALSE;
	privateSem = NULL;
    }
    else {
	/* dequeue the first semaphore on the list of waiting processes */
	privateSem = latchPtr->semList;
	latchPtr->semList = privateSem->nextSem;
	/* bolo: moved below  ... SendSem(&privateSem->semid); */
    }

    /* 
     * end of critical section.
     * Release the latch's gate.  SpinUnlock(L.gate);
     */
#if defined(TESTANDSET)
    testandset_release(&latchPtr->sync);
#else
	//james 161007
	atomic_flag_clear(&latchPtr->sync);
	//james 161007
#endif

    if (privateSem)
	SendSem(&privateSem->semid);
/*
    printf("proc %d leaves C.S. in RL\n",procNum);
*/
}

