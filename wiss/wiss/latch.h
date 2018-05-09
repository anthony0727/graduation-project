
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

#include <stdatomic.h>

/* type declarations for latches */

/* The synchronization mechanism asssumes that each active */
/* process has its own system V semaphore.  Latches */
/* are used to synchronize processes on shared data structures  */
/* semaphores are acquired via a call to system V semget */

#define		MAXPROCS	32  /* maximum number of concurrent processes */
//james 161010
#define		MAXTHREADS	16	/* maximum number of concurrent threads */
//james 161010

#if !defined(NOTESTANDSET)
#if defined(sun) || defined(vax)
#define	TESTANDSET
#endif
#endif

typedef struct semNode {
	int	   	semid;  /* semaphore id */
	//james 161010
	atomic_flag sync;
	//james 161010
	struct semNode  *nextSem;  /* to thread semaphores */
} SEMNODE;

typedef struct {
	char	busy;	/* true if latch is busy */
#if defined(TESTANDSET)
	char	sync;
#else
	atomic_flag sync;
#endif
	SEMNODE	*semList;  /* list of waiting processes */
} LATCH;
